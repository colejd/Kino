#include "CalibrationState.hpp"

CalibrationState::CalibrationState(int board_width, int board_height, int capturesRequired, float square_size) {
	this->board_size = Size(board_width, board_height);
	this->capturesRequired = capturesRequired;
	this->square_size = square_size;

	Reset();

}


void CalibrationState::Reset() {
	complete = false;

	object_points.clear();
	image_points.clear();
	corners.clear();
	rvecs.clear();
	tvecs.clear();

	K = cv::Mat();
	D = cv::Mat();

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

	bool found = cv::findChessboardCorners(in, board_size, corners, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
	if (found) {
		cornerSubPix(gray, corners, cv::Size(5, 5), cv::Size(-1, -1),
			TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
		drawChessboardCorners(out, board_size, corners, found);
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
		// Do a new capture
		numCaptures += 1;

		vector< Point3f > obj;
		for (int i = 0; i < board_size.height; i++)
			for (int j = 0; j < board_size.width; j++)
				obj.push_back(Point3f((float)j * square_size, (float)i * square_size, 0));

		image_points.push_back(corners);
		object_points.push_back(obj);

	}

	if (numCaptures == capturesRequired) {
		// We have enough samples -- finish calibrating.
		int flag = 0;
		flag |= CV_CALIB_FIX_K4;
		flag |= CV_CALIB_FIX_K5;
		calibrateCamera(object_points, image_points, in.size(), K, D, rvecs, tvecs, flag);
		complete = true;
	}


}


void CalibrationState::UndistortImage(cv::InputArray in, cv::OutputArray out) {
	if (!mapsCreated) {
		initUndistortRectifyMap(K, D, cv::Mat(), K, in.size(), CV_32FC1, map1, map2);
		mapsCreated = true;
	}

	remap(in, out, map1, map2, INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0, 1));
}


bool CalibrationState::LoadFromFile(string path) {
	// Populates the model from a file.
	bool success = false;

	// TODO: Add functionality

	complete = success;
	return success;
}


bool CalibrationState::SaveToFile(string fileName) {
	// TODO: Add functionality

	return false;
}


