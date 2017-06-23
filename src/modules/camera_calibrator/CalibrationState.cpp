#include "CalibrationState.hpp"

CalibrationState::CalibrationState(int board_width, int board_height, int capturesRequired, float squareSize) {
	this->boardSize = Size(board_width, board_height);
	this->capturesRequired = capturesRequired;
	this->squareSize = squareSize;

	Reset();

}


void CalibrationState::Reset() {
	complete = false;

	objectPoints.clear();
	imagePoints.clear();
	corners.clear();
	rvecs.clear();
	tvecs.clear();
	reprojErrs.clear();

	cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = 1; // Set aspect ratio to 1 (for focal length)
	distortionCoeffs = cv::Mat::zeros(8, 1, CV_64F);

	mapsCreated = false;
	map1 = cv::Mat();
	map2 = cv::Mat();

	lastFrameTime = 0.0;
	secondsSinceLastCapture = 0.0;
	numCaptures = 0;
}


void CalibrationState::ProcessImage(cv::InputArray in, cv::InputOutputArray out) {

	cv::Mat gray;
	cv::cvtColor(in, gray, CV_BGR2GRAY);

	bool found = cv::findChessboardCorners(in, boardSize, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE); //CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	if (found) {
		cornerSubPix(gray, corners, cv::Size(5, 5), cv::Size(-1, -1),
			TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
		drawChessboardCorners(out, boardSize, corners, found);
	}

	bool timeForNewCheckerboard = false;
	float time = ofGetElapsedTimef();
	secondsSinceLastCapture += time - lastFrameTime;
	lastFrameTime = time;
	if (secondsSinceLastCapture > secondsBetweenCaptures) {
		secondsSinceLastCapture = 0.0;
		timeForNewCheckerboard = true;
	}
	
	if (found && numCaptures < capturesRequired && timeForNewCheckerboard) {
		// Use the current frame as an input image for the calibrator
		numCaptures += 1;

		vector< Point3f > obj;
		for (int i = 0; i < boardSize.height; i++)
			for (int j = 0; j < boardSize.width; j++)
				obj.push_back(Point3f((float)j * squareSize, (float)i * squareSize, 0));

		imagePoints.push_back(corners);
		objectPoints.push_back(obj);

	}

	if (numCaptures == capturesRequired) {
		// We have enough samples -- finish calibrating.
		int flag = 0;
		flag |= CV_CALIB_FIX_K4;
		flag |= CV_CALIB_FIX_K5;
		flag |= CV_CALIB_FIX_ASPECT_RATIO;
		double rms = calibrateCamera(objectPoints, imagePoints, in.size(), cameraMatrix, distortionCoeffs, rvecs, tvecs, flag);

		Kino::app_log.AddLog("Reprojection error reported by calibrateCamera: %f\n", rms);

		// Calculate error
		reprojErrs.resize(objectPoints.size());
		vector<Point2f> imagePointsProjected;
		size_t totalPoints = 0;
		double totalErr = 0;
		for (size_t i = 0; i < objectPoints.size(); ++i) {
			projectPoints(objectPoints[i], rvecs[i], tvecs[i], cameraMatrix, distortionCoeffs, imagePointsProjected);
			double err = norm(imagePoints[i], imagePointsProjected, NORM_L2);
			
			size_t n = objectPoints[i].size();
			reprojErrs[i] = (float)std::sqrt(err * err / n);
			totalErr += err * err;
			totalPoints += n;
		}
		reprojectionError = sqrt(totalErr / totalPoints);
		Kino::app_log.AddLog("Reprojection error computed: %f\n", reprojectionError);

		if (checkRange(cameraMatrix) && checkRange(distortionCoeffs)) {
			complete = true; // Mark the calibration as finished
			SaveToFile();
		}
		else {
			Kino::app_log.AddLog("Calibration contained invalid data. Please retry.");
			Reset();
		}
	}


}


void CalibrationState::UndistortImage(cv::InputArray in, cv::OutputArray out) {
	if (!mapsCreated) {
		initUndistortRectifyMap(cameraMatrix, distortionCoeffs, cv::Mat(), 
			getOptimalNewCameraMatrix(cameraMatrix, distortionCoeffs, in.size(), 1, in.size(), 0), 
			in.size(), CV_32FC1, map1, map2);
		mapsCreated = true;
	}

	remap(in, out, map1, map2, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0, 1));
}


bool CalibrationState::LoadFromFile() {
	// Populates the model from a file.

	// TODO: Add functionality
	string path = ofToDataPath("calibration/" + unique_id + ".yml");
	FileStorage fs;
	if (!fs.open(path, FileStorage::READ)) {
		complete = false;
		return false;
	}

	boardSize = cv::Size(fs["board_width"], fs["board_height"]);

	fs["squareSize"] >> squareSize;
	fs["camera_matrix"] >> cameraMatrix;
	fs["distortion_coefficients"] >> distortionCoeffs;
	fs["avg_reprojection_error"] >> reprojectionError;

	//fs["per_view_reprojection_errors"] >> reprojErrs;
	//fs["extrinsic_parameters"] >> ;
	//fs["image_points"] >> imagePoints;

	complete = true;
	return true;
}


bool CalibrationState::SaveToFile() {
	string path = ofToDataPath("calibration/" + unique_id + ".yml");
	FileStorage fs(path, FileStorage::WRITE);

	time_t tm;
	time(&tm);
	struct tm *t2 = localtime(&tm);
	char buf[1024];
	strftime(buf, sizeof(buf), "%c", t2);

	fs << "calibration_time" << buf;

	if (!rvecs.empty() || !reprojErrs.empty())
		fs << "nr_of_frames" << (int)std::max(rvecs.size(), reprojErrs.size());
	fs << "board_width" << boardSize.width;
	fs << "board_height" << boardSize.height;
	fs << "squareSize" << squareSize;

	fs << "camera_matrix" << cameraMatrix;
	fs << "distortion_coefficients" << distortionCoeffs;

	fs << "avg_reprojection_error" << reprojectionError;
	if (!reprojErrs.empty()) {
		fs << "per_view_reprojection_errors" << Mat(reprojErrs);
	}

	if (!rvecs.empty() && !tvecs.empty()) {
		CV_Assert(rvecs[0].type() == tvecs[0].type());
		Mat bigmat((int)rvecs.size(), 6, CV_MAKETYPE(rvecs[0].type(), 1));
		bool needReshapeR = rvecs[0].depth() != 1 ? true : false;
		bool needReshapeT = tvecs[0].depth() != 1 ? true : false;

		for (size_t i = 0; i < rvecs.size(); i++) {
			Mat r = bigmat(Range(int(i), int(i + 1)), Range(0, 3));
			Mat t = bigmat(Range(int(i), int(i + 1)), Range(3, 6));

			if (needReshapeR)
				rvecs[i].reshape(1, 1).copyTo(r);
			else {
				//*.t() is MatExpr (not Mat) so we can use assignment operator
				CV_Assert(rvecs[i].rows == 3 && rvecs[i].cols == 1);
				r = rvecs[i].t();
			}

			if (needReshapeT)
				tvecs[i].reshape(1, 1).copyTo(t);
			else {
				CV_Assert(tvecs[i].rows == 3 && tvecs[i].cols == 1);
				t = tvecs[i].t();
			}
		}
		fs.writeComment("a set of 6-tuples (rotation vector + translation vector) for each view");
		fs << "extrinsic_parameters" << bigmat;
	}

	if (!imagePoints.empty()) {
		Mat imagePtMat((int)imagePoints.size(), (int)imagePoints[0].size(), CV_32FC2);
		for (size_t i = 0; i < imagePoints.size(); i++) {
			Mat r = imagePtMat.row(int(i)).reshape(2, imagePtMat.cols);
			Mat imgpti(imagePoints[i]);
			imgpti.copyTo(r);
		}
		fs << "image_points" << imagePtMat;
	}

	return true;
}



