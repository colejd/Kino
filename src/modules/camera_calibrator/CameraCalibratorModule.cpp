#include "CameraCalibratorModule.hpp"

CameraCalibratorModule::CameraCalibratorModule() {
	// Automatically set mode to stereo unless demo mode
	bool demoMode = ConfigHandler::GetValue("DEMO_SETTINGS.ACTIVE", false).asBool();
	if (demoMode) {
		currentMode = Mode::INDIVIDUAL;
	}
	else {
		currentMode = Mode::STEREO;
	}
}

CameraCalibratorModule::~CameraCalibratorModule() {

}

/**
Registers camera captures for calibration and distortion. Builds the map.
*/
void CameraCalibratorModule::RegisterCameras(unique_ptr<CameraCapture> const& leftCapture, unique_ptr<CameraCapture> const& rightCapture) {
	InitCalibrationState(leftCapture, LEFT_ID);
	InitCalibrationState(rightCapture, RIGHT_ID);

	// Create stereo capture
	if (leftCapture->IsInitialized() && rightCapture->IsInitialized()) {
		stereoCalibration = StereoCalibrationState();
		stereoCalibration.unique_id = leftCapture->DeviceName() + "_STEREO";
		stereoCalibration.LoadFromFile();
		if (!stereoCalibration.complete) {
			Kino::app_log.AddLog("No calibration for %s exists.\n", stereoCalibration.unique_id.c_str());
		}
	}
}

void CameraCalibratorModule::InitCalibrationState(unique_ptr<CameraCapture> const& cap, string id) {
	calibrations[id] = CalibrationState();

	if (cap->IsInitialized()) {
		calibrations[id].hasCapture = true;
		// Load the calibration from the disk if it exists.
		calibrations[id].unique_id = cap->DeviceName() + "_" + id;
		calibrations[id].LoadFromFile();
		if (!calibrations[id].complete) {
			Kino::app_log.AddLog("No calibration for %s exists.\n", calibrations[id].unique_id.c_str());
		}
	}
}

void CameraCalibratorModule::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	moduleCanRunStereo = !inLeft.empty() && !inRight.empty();
	// Switch to individual if we're on stereo and we don't have two cameras
	if (!moduleCanRunStereo) currentMode = Mode::INDIVIDUAL;

	if (IsEnabled()) {
		TS_SCOPE("Camera Calibrator");

		inLeft.copyTo(lastLeftMat);
		inRight.copyTo(lastRightMat);

		if (currentMode == Mode::INDIVIDUAL) {

			if (!inLeft.empty()) {
				CalibrationState* leftCalib = &(calibrations[LEFT_ID]);
				// If the model is fully formed, do the distortion.
				if (leftCalib->complete) {
					TS_SCOPE("Undistort");
					leftCalib->UndistortImage(inLeft, outLeft);
				}
				// Otherwise, run calibration.
				else {
					// Only run calibration if the ID given matches the desired ID
					if (currentlyCalibratingID == LEFT_ID) {
						TS_SCOPE("Process Calibration Image");
						leftCalib->IngestImageForCalibration(inLeft, outLeft);
						if (leftCalib->complete) {
							StopCalibrating(LEFT_ID); // Quit the calibration
						}
					}
				}
			}

			if (!inRight.empty()) {
				CalibrationState* rightCalib = &(calibrations[RIGHT_ID]);
				// If the model is fully formed, do the distortion.
				if (rightCalib->complete) {
					TS_SCOPE("Undistort");
					rightCalib->UndistortImage(inRight, outRight);
				}
				// Otherwise, run calibration.
				else {
					// Only run calibration if the ID given matches the desired ID
					if (currentlyCalibratingID == RIGHT_ID) {
						TS_SCOPE("Process Calibration Image");
						rightCalib->IngestImageForCalibration(inRight, outRight);
						if (rightCalib->complete) {
							StopCalibrating(RIGHT_ID); // Quit the calibration
						}
					}
				}
			}
		}

		else if (currentMode == Mode::STEREO) {
			if (captureMode) {
				DrawCheckerboardPreview(inLeft, outLeft);
				DrawCheckerboardPreview(inRight, outRight);
			}
			else if (stereoCalibration.complete) {
				// Rectify
				TS_SCOPE("Undistort Stereo");
				stereoCalibration.UndistortImage(inLeft, inRight, outLeft, outRight);

				if (drawEpipolarLines) {
					VisualizeEpipolarLines(outLeft, outRight, outLeft, outRight);
				}

			}
		}


	}
}

void CameraCalibratorModule::DrawGUI() {
	if (showGUI) {
		ImGui::Begin(GetName().c_str(), &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Enabled", &enabled);

		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{

			// Do mode dropdown
			int mode = (int)currentMode;
			char* items = "Individual\0\0";
			if (moduleCanRunStereo) items = "Individual\0Stereo\0\0"; // Show Stereo option only if it's possible to use it
			ImGui::Combo("Mode", &mode, items);

			if (mode == (int)Mode::INDIVIDUAL) {
				currentMode = Mode::INDIVIDUAL;
			}
			else {
				currentMode = Mode::STEREO;
			}

			// Draw GUI for INDIVIDUAL
			if (currentMode == Mode::INDIVIDUAL) {
				// Draw info about left calibration
				bool leftHasCapture = calibrations[LEFT_ID].hasCapture;
				if (leftHasCapture) {
					ImGui::Spacing();
					DrawCalibrationStatePanel(LEFT_ID);
				}

				// Draw info about right calibration
				bool rightHasCapture = calibrations[RIGHT_ID].hasCapture;
				if (rightHasCapture) {
					ImGui::Spacing();
					DrawCalibrationStatePanel(RIGHT_ID);
				}
			}

			// Draw GUI for STEREO
			if (currentMode == Mode::STEREO) {
				ImGui::Spacing();
				DrawStereoCalibrationPanel();

				ImGui::Checkbox("Capture Mode", &captureMode);
				if (captureMode && ImGui::Button("Save Capture Pairs")) {
					string basePath = ofToDataPath("calibration/images/" + stereoCalibration.unique_id + "/");

					// Left image
					string basePathLeft = basePath + "LEFT/";
					ofDirectory::createDirectory(basePathLeft, false, true);
					imwrite(basePathLeft + "Capture" + std::to_string(capCount) + ".png", lastLeftMat);

					// Right image
					string basePathRight = basePath + "RIGHT/";
					ofDirectory::createDirectory(basePathRight, false, true);
					imwrite(basePathRight + "Capture" + std::to_string(capCount) + ".png", lastRightMat);

					capCount += 1;
					Kino::app_log.AddLog("Stereo image pair written to LEFT and RIGHT in data/calibration/images/");
				}

			}
			

			ImGui::Checkbox("Draw Feature Matches", &drawEpipolarLines);
			

		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}

void CameraCalibratorModule::DrawCalibrationStatePanel(string id) {

	CalibrationState& calibration = calibrations[id];

	bool calibratingOther = (currentlyCalibratingID != id && currentlyCalibratingID != "");
	if (calibratingOther) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style

	ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
	ImGui::BeginChild((id + "Sub").c_str(), ImVec2(250, 150), true);
	{
		// Header
		ImGui::Text(id.c_str());

		ImVec4 calibratingColor;
		string calibratingString;
		if (calibration.complete) {
			calibratingColor = ImVec4(0.0, 1.0, 0.0, 1.0);
			calibratingString = "Calibrated";
		}
		else if (currentlyCalibratingID == id) {
			calibratingColor = ImVec4(1.0, 1.0, 0.0, 1.0);
			calibratingString = "Calibrating...";
		}
		else {
			calibratingColor = ImVec4(1.0, 0.0, 0.0, 1.0);
			calibratingString = "Uncalibrated";
		}
		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(calibratingString.c_str()).x - 10);
		ImGui::TextColored(calibratingColor, calibratingString.c_str());

		ImGui::Separator();
		ImGui::Spacing();

		// Body
		if (currentlyCalibratingID == "") {
			if (ImGui::Button("Calibrate")) {
				StartCalibrating(id);
			}
		}
		else if (currentlyCalibratingID == id) {
			if (ImGui::Button("Stop Calibrating")) {
				StopCalibrating(id);
			}
		}

		ImGui::Text("%-20s (%i x %i)", "Board Size:", calibration.boardSize.width, calibration.boardSize.height);
		ImGui::Text("%-20s %.2fmm", "Square Size:", calibration.squareSize);
		ImGui::Text("%-20s %.3f", "Reprojection Error:", calibration.reprojectionError);
		ShowHelpMarker("Should be as close to 0 as possible.");

		if (currentlyCalibratingID == id) {
			ImGui::Text("%-20s %d / %d", "Captures:", calibration.numCaptures, calibration.capturesRequired);
		}

		if (ImGui::Button("Save Capture")) {
			string basePath = ofToDataPath("calibration/images/" + calibration.unique_id + "/");

			ofDirectory::createDirectory(basePath, false, true);
			bool isLeft = (id == LEFT_ID);
			imwrite(basePath + "Capture" + std::to_string(isLeft ? leftCapCount : rightCapCount) + ".png", isLeft ? lastLeftMat : lastRightMat);
			if (isLeft) leftCapCount += 1;
			else rightCapCount += 1;
			
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

	if (calibratingOther) ImGui::PopStyleVar(); //Pop disabled style

}

void CameraCalibratorModule::DrawStereoCalibrationPanel() {
	ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
	ImGui::BeginChild("STEREO Sub", ImVec2(250, 150), true);
	{
		// Header
		ImGui::Text("STEREO");

		ImVec4 calibratingColor;
		string calibratingString;
		if (stereoCalibration.complete) {
			calibratingColor = ImVec4(0.0, 1.0, 0.0, 1.0);
			calibratingString = "Calibrated";
		}
		else {
			calibratingColor = ImVec4(1.0, 0.0, 0.0, 1.0);
			calibratingString = "Uncalibrated";
		}
		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(calibratingString.c_str()).x - 10);
		ImGui::TextColored(calibratingColor, calibratingString.c_str());

		ImGui::Separator();
		ImGui::Spacing();


		// Body
		if (currentlyCalibratingID == "") {
			if (ImGui::Button("Calibrate")) {
				//Calibrate immediately with image set
				stereoCalibration.CalibrateWithImageSet();

			}
		}

		ImGui::Checkbox("Calibrate Individually", &(stereoCalibration.calibrateIndividually));

		ImGui::Text("%-20s %.2fmm", "Unit Size:", stereoCalibration.squareSize);
		ImGui::Text("%-20s %.3f", "Reprojection Error:", stereoCalibration.reprojectionError);
		ShowHelpMarker("Should be as close to 0 as possible.");


	}
	ImGui::EndChild();
	ImGui::PopStyleVar();
}

void CameraCalibratorModule::StartCalibrating(string id) {
	calibrations[id].Reset();
	currentlyCalibratingID = id;
}


void CameraCalibratorModule::StopCalibrating(string id) {
	currentlyCalibratingID = "";
}


void CameraCalibratorModule::DrawCheckerboardPreview(InputArray in, OutputArray out) {
	cv::Mat gray;
	cv::cvtColor(in, gray, CV_BGR2GRAY);
	vector<Point2f> corners;
	bool found = cv::findChessboardCorners(in, stereoCalibration.boardSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE); //CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	if (found) {
		cv::Mat dst;
		in.copyTo(dst);
		cornerSubPix(gray, corners, cv::Size(5, 5), cv::Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
		drawChessboardCorners(dst, stereoCalibration.boardSize, corners, found);
		dst.copyTo(out);
	}
}


/**
Adapted from https://stackoverflow.com/a/31835365

Uses feature matching to find common elements between the two image sides, then
draws the epipolar lines using the found points. The calibration is correct if
the lines are horizontal.

We can calculate and show the epilines based on this:
http://docs.opencv.org/master/da/de9/tutorial_py_epipolar_geometry.html

*/
void CameraCalibratorModule::VisualizeEpipolarLines(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {

	std::vector<KeyPoint> keypointsLeft, keypointsRight;

	//-- Step 1: Detect the keypoints using SURF Detector
	detector->detect(inLeft, keypointsLeft);
	detector->detect(inRight, keypointsRight);

	//-- Step 2: Calculate descriptors (feature vectors)
	UMat descriptorsLeft, descriptorsRight;
	extractor->compute(inLeft, keypointsLeft, descriptorsLeft);
	extractor->compute(inRight, keypointsRight, descriptorsRight);

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	std::vector< DMatch > matches;
	matcher->match(descriptorsLeft, descriptorsRight, matches);

	double max_dist = 0;
	double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for (int i = 0; i < descriptorsLeft.rows; i++) {
		double dist = matches[i].distance;
		if (dist < min_dist) min_dist = dist;
		if (dist > max_dist) max_dist = dist;
	}

	//printf("-- Max dist : %f \n", max_dist);
	//printf("-- Min dist : %f \n", min_dist);

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	std::vector< DMatch > good_matches;

	for (DMatch m : matches) {
		bool pass = false;
		pass = m.distance < 3 * min_dist;
		// Filter for similar Y coords
		//pass = std::abs(keypointsLeft[m.queryIdx].pt.y - keypointsRight[m.trainIdx].pt.y) < 3;
		if (pass) {
			good_matches.push_back(m);
		}
	}

	Mat img_matches;

	//drawMatches(inLeft, keypointsLeft, inRight, keypointsRight, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	//-- Localize the object
	std::vector<Point2f> leftPoints;
	std::vector<Point2f> rightPoints;

	for (int i = 0; i < good_matches.size(); i++) {
		//-- Get the keypoints from the good matches
		leftPoints.push_back(keypointsLeft[good_matches[i].queryIdx].pt);
		rightPoints.push_back(keypointsRight[good_matches[i].trainIdx].pt);
	}

	cv::Mat leftMat, rightMat;
	inLeft.copyTo(leftMat);
	inRight.copyTo(rightMat);

	img_matches = StereoCalibrationState::drawEpipolarLines<float, float>("", stereoCalibration.F, leftMat, rightMat, leftPoints, rightPoints);

	// Copy results to output (split img_matches in half vertically)
	img_matches(cv::Rect(0, 0, img_matches.cols / 2, img_matches.rows)).copyTo(outLeft);
	img_matches(cv::Rect(img_matches.cols / 2, 0, img_matches.cols / 2, img_matches.rows)).copyTo(outRight);

	//-- Get the corners from the image_1 ( the object to be "detected" )
	//std::vector<Point2f> cornersLeft(4);
	//cornersLeft[0] = cvPoint(0, 0); cornersLeft[1] = cvPoint(left.cols, 0);
	//cornersLeft[2] = cvPoint(left.cols, left.rows); cornersLeft[3] = cvPoint(0, left.rows);
	//std::vector<Point2f> cornersRight(4);

	//Mat H = findHomography(leftPoints, rightPoints, CV_RANSAC);
	//perspectiveTransform(obj_corners, scene_corners, H);

	//-- Draw lines between the corners (the mapped object in the scene - image_2 )
	//line(img_matches, scene_corners[0] + Point2f(left.cols, 0), scene_corners[1] + Point2f(left.cols, 0), Scalar(0, 255, 0), 4);
	//line(img_matches, scene_corners[1] + Point2f(left.cols, 0), scene_corners[2] + Point2f(left.cols, 0), Scalar(0, 255, 0), 4);
	//line(img_matches, scene_corners[2] + Point2f(left.cols, 0), scene_corners[3] + Point2f(left.cols, 0), Scalar(0, 255, 0), 4);
	//line(img_matches, scene_corners[3] + Point2f(left.cols, 0), scene_corners[0] + Point2f(left.cols, 0), Scalar(0, 255, 0), 4);

	//imshow("Good Matches & Object detection", img_matches);
}