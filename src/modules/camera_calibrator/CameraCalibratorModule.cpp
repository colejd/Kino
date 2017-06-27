#include "CameraCalibratorModule.hpp"

CameraCalibratorModule::CameraCalibratorModule() {
	// TODO: Automatically set mode to stereo unless demo mode
}

CameraCalibratorModule::~CameraCalibratorModule() {

}

/**
Registers camera captures for calibration and distortion. Builds the map.
*/
void CameraCalibratorModule::RegisterCameras(unique_ptr<CameraCapture> const& leftCapture, unique_ptr<CameraCapture> const& rightCapture) {
	InitCalibrationState(leftCapture, "LEFT");
	InitCalibrationState(rightCapture, "RIGHT");

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

void CameraCalibratorModule::ProcessFrame(cv::InputArray in, cv::InputOutputArray out, string id) {
	if (IsEnabled()) {
		TS_SCOPE("Camera Calibrator");

		in.copyTo(id == "LEFT" ? lastLeftMat : lastRightMat);

		if (currentMode == Mode::INDIVIDUAL) {

			CalibrationState* calibration = &(calibrations[id]);

			// If the model is fully formed, do the distortion.
			if (calibration->complete) {
				TS_SCOPE("Undistort");
				calibration->UndistortImage(in, out);
			}

			// Otherwise, run calibration.
			else {
				// Only run calibration if the ID given matches the desired ID
				if (id == currentlyCalibratingID) {
					TS_SCOPE("Process Calibration Image");
					calibration->IngestImageForCalibration(in, out);
					if (calibration->complete) {
						StopCalibrating(id); // Quit the calibration
					}
				}
			}
		}

		else if (currentMode == Mode::STEREO) {
			if (captureMode) {
				cv::Mat gray;
				cv::cvtColor(in, gray, CV_BGR2GRAY);
				vector<Point2f> corners;
				bool found = cv::findChessboardCorners(in, stereoCalibration.boardSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE); //CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
				if (found) {
					cornerSubPix(gray, corners, cv::Size(5, 5), cv::Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
					drawChessboardCorners(out, stereoCalibration.boardSize, corners, found);
				}
			}
			else if (stereoCalibration.complete) {
				// Rectify
				TS_SCOPE("Undistort Stereo");
				stereoCalibration.UndistortImage(in, out, id);

			}
		}


	}
}

void CameraCalibratorModule::DrawGUI() {
	if (showGUI) {

		ImGui::Begin("Camera Calibrator", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{

			// Do mode dropdown
			int mode = (int)currentMode;
			ImGui::Combo("Mode", &mode, "INDIVIDUAL\0STEREO\0\0");
			if (mode == (int)Mode::INDIVIDUAL) {
				currentMode = Mode::INDIVIDUAL;
			}
			else {
				currentMode = Mode::STEREO;
			}

			// Draw GUI for INDIVIDUAL
			if (currentMode == Mode::INDIVIDUAL) {
				// Draw info about left calibration
				bool leftHasCapture = calibrations["LEFT"].hasCapture;
				if (leftHasCapture) {
					ImGui::Spacing();
					DrawCalibrationStatePanel("LEFT");
				}

				// Draw info about right calibration
				bool rightHasCapture = calibrations["RIGHT"].hasCapture;
				if (rightHasCapture) {
					ImGui::Spacing();
					DrawCalibrationStatePanel("RIGHT");
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
					Kino::app_log.AddLog("Stereo image pair written to data/calibration/images/");
				}

			}
			

			

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
			bool isLeft = (id == "LEFT");
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