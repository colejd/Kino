#include "StereoCalibrationState.hpp"

StereoCalibrationState::StereoCalibrationState(int board_width, int board_height, int capturesRequired, float squareSize) {
	this->boardSize = Size(board_width, board_height);
	this->capturesRequired = capturesRequired;
	this->squareSize = squareSize;

	Reset();

}


void StereoCalibrationState::Reset() {
	complete = false;

	objectPoints.clear();
	imagePointsLeft.clear();
	imagePointsRight.clear();
	cornersLeft.clear();
	cornersRight.clear();

	// Variables needed to restore from disk
	cameraMatrixLeft = Mat();
	cameraMatrixRight = Mat();
	distortionCoeffsLeft = Mat();
	distortionCoeffsRight = Mat();

	T = Vec3d();
	R = Mat();
	F = Mat();
	E = Mat();

	R1 = Mat();
	R2 = Mat();
	P1 = Mat();
	P2 = Mat();
	Q = Mat();
}

bool StereoCalibrationState::QueueImages(cv::InputArray inLeft, cv::InputArray inRight) {

	bool foundLeft = cv::findChessboardCorners(inLeft, boardSize, cornersLeft, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE); //CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	if (foundLeft) {
		cv::Mat leftGray;
		cv::cvtColor(inLeft, leftGray, CV_BGR2GRAY);

		cornerSubPix(leftGray, cornersLeft, cv::Size(5, 5), cv::Size(-1, -1),
						TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
	}

	bool foundRight = cv::findChessboardCorners(inRight, boardSize, cornersRight, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FAST_CHECK | CV_CALIB_CB_NORMALIZE_IMAGE); //CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	if (foundRight) {
		cv::Mat rightGray;
		cv::cvtColor(inRight, rightGray, CV_BGR2GRAY);

		cornerSubPix(rightGray, cornersRight, cv::Size(5, 5), cv::Size(-1, -1),
						TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
	}
	
	
	if(foundLeft && foundRight){
		vector< Point3f > obj;
		for (int i = 0; i < boardSize.height; i++)
			for (int j = 0; j < boardSize.width; j++)
				obj.push_back(Point3f((float)j * squareSize, (float)i * squareSize, 0));

		objectPoints.push_back(obj);
		imagePointsLeft.push_back(cornersLeft);
		imagePointsRight.push_back(cornersRight);

		return true;
	}

	return false;
}


/**
This calibration state will immediately populate itself using the images located at a special directory.
Probably /data/calibration/images/<unique_id>/
*/
void StereoCalibrationState::CalibrateWithImageSet() {
	Reset();

	string basePath = ofToDataPath("calibration/images/" + unique_id + "/");

	ofDirectory leftDir(basePath + "LEFT");
	ofDirectory rightDir(basePath + "RIGHT");

	ValidateDir(leftDir);
	ValidateDir(rightDir);

	vector<ofFile> leftFiles = leftDir.getFiles();
	vector<ofFile> rightFiles = rightDir.getFiles();

	if (leftFiles.size() != rightFiles.size()) {
		Kino::app_log.AddLog("[StereoCalibrationState] Number of files for LEFT (%i) does not match RIGHT (%i)\n", leftFiles.size(), rightFiles.size());
		return;
	}

	Kino::app_log.AddLog("[StereoCalibrationState] Calibrating with %i images...\n", leftFiles.size());

	cv::Size size;
	for (int i = 0; i < leftFiles.size(); i++) {
		cv::Mat leftMat = imread(leftFiles[i].getAbsolutePath(), CV_LOAD_IMAGE_COLOR);
		cv::Mat rightMat = imread(rightFiles[i].getAbsolutePath(), CV_LOAD_IMAGE_COLOR);

		size = cv::Size(leftMat.cols, leftMat.rows);
		bool didQueue = QueueImages(leftMat, rightMat);
		if (!didQueue) {
			Kino::app_log.AddLog("[StereoCalibrationState] Could not find a checkerboard in set %i.\n", i);
			//Kino::app_log.AddLog("[StereoCalibrationState] Could not find a checkerboard in the given image: %s\n", file.getFileName().c_str());
		}

	}

	// Use accumulated data to finalize calibration.

	// Figure out camera calibration params individually (more accurate)
	vector<Mat> rvecsLeft, tvecsLeft, rvecsRight, tvecsRight;
	int calibFlag = 0;
	calibFlag |= CV_CALIB_FIX_K4;
	calibFlag |= CV_CALIB_FIX_K5;
	calibFlag |= CV_CALIB_FIX_ASPECT_RATIO;
	double rms;
	rms = calibrateCamera(objectPoints, imagePointsLeft, size, cameraMatrixLeft, distortionCoeffsLeft, rvecsLeft, tvecsLeft, calibFlag);
	Kino::app_log.AddLog("Reprojection error reported by left calibration: %f\n", rms);
	rms = calibrateCamera(objectPoints, imagePointsRight, size, cameraMatrixRight, distortionCoeffsRight, rvecsRight, tvecsRight, calibFlag);
	Kino::app_log.AddLog("Reprojection error reported by right calibration: %f\n", rms);

	// Stereo calibrate using individual calibrations as inputs
	int flag = 0;
	flag |= CV_CALIB_FIX_INTRINSIC; // The default value
	reprojectionError = stereoCalibrate(objectPoints, imagePointsLeft, imagePointsRight, cameraMatrixLeft, distortionCoeffsLeft, cameraMatrixRight, distortionCoeffsRight, size, R, T, E, F);

	Kino::app_log.AddLog("Reprojection error reported by stereo calibration: %f\n", reprojectionError);


	stereoRectify(cameraMatrixLeft, distortionCoeffsLeft, cameraMatrixRight, distortionCoeffsRight, size, R, T, R1, R2, P1, P2, Q, flag);

	mapsCreated = false;

	if (checkRange(cameraMatrixLeft) && checkRange(distortionCoeffsLeft) && 
		checkRange(cameraMatrixRight) && checkRange(distortionCoeffsRight)) {

		complete = true; // Mark the calibration as finished
		SaveToFile();
	}
	else {
		Kino::app_log.AddLog("Calibration contained invalid data. Please retry.");
		Reset();
	}



}


void StereoCalibrationState::UndistortImage(cv::InputArray in, cv::OutputArray out, string id) {
	if (!mapsCreated) {
		if (!R1.empty() && !R2.empty() && !P1.empty() && !P2.empty()) {
			initUndistortRectifyMap(cameraMatrixLeft, distortionCoeffsLeft, R1, P1, in.size(), CV_32FC1, map1_left, map2_left);
			initUndistortRectifyMap(cameraMatrixRight, distortionCoeffsRight, R2, P2, in.size(), CV_32FC1, map1_right, map2_right);
			mapsCreated = true;
		}
	}
	if (id == "LEFT") {
		remap(in, out, map1_left, map2_left, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0, 1));
	}
	else {
		remap(in, out, map1_right, map2_right, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0, 1));
	}
}


bool StereoCalibrationState::LoadFromFile() {
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
	fs["avg_reprojection_error"] >> reprojectionError;

	// Intrinsics
	fs["cameraMatrixLeft"] >> cameraMatrixLeft;
	fs["distortionCoeffsLeft"] >> distortionCoeffsLeft;
	fs["cameraMatrixRight"] >> cameraMatrixRight;
	fs["distortionCoeffsRight"] >> distortionCoeffsRight;

	fs["T"] >> T;
	fs["R"] >> R;
	fs["E"] >> E;
	fs["F"] >> F;

	fs["R1"] >> R1;
	fs["R2"] >> R2;
	fs["P1"] >> P1;
	fs["P2"] >> P2;
	fs["Q"] >> Q;


	complete = true;
	return true;
}


bool StereoCalibrationState::SaveToFile() {
	string path = ofToDataPath("calibration/" + unique_id + ".yml");
	FileStorage fs(path, FileStorage::WRITE);

	time_t tm;
	time(&tm);
	struct tm *t2 = localtime(&tm);
	char buf[1024];
	strftime(buf, sizeof(buf), "%c", t2);

	fs << "calibration_time" << buf;

	fs << "board_width" << boardSize.width;
	fs << "board_height" << boardSize.height;
	fs << "squareSize" << squareSize;

	fs << "cameraMatrixLeft" << cameraMatrixLeft;
	fs << "distortionCoeffsLeft" << distortionCoeffsLeft;

	fs << "cameraMatrixRight" << cameraMatrixRight;
	fs << "distortionCoeffsRight" << distortionCoeffsRight;

	fs << "R" << R;
	fs << "T" << T;
	fs << "E" << E;
	fs << "F" << F;

	fs << "R1" << R1;
	fs << "R2" << R2;
	fs << "P1" << P1;
	fs << "P2" << P2;
	fs << "Q" << Q;

	fs << "avg_reprojection_error" << reprojectionError;

	return true;
}

bool StereoCalibrationState::ValidateDir(ofDirectory dir) {
	if (!dir.exists()) {
		dir.create(true);
	}

	vector<ofFile> files = dir.getFiles();
	if (files.size() == 0) {
		Kino::app_log.AddLog("[StereoCalibrationState] There are no images in the directory %s\n", dir.path().c_str());
		return false;
	}

	return true;
}




