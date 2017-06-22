#include "CalibrationState.hpp"

CalibrationState::CalibrationState(int board_width, int board_height, int num_imgs, float square_size) {

	this->board_width = board_width;
	this->board_height = board_height;
	this->board_size = Size(board_width, board_height);
	this->num_imgs = num_imgs;
	this->square_size = square_size;

	this->board_n = board_width * board_height;

	//found = false;
	numCaptures = 0;

	//vector< Point3f > obj;
	//// Populate object points (units will be millimeters)
	//for (int i = 0; i < board_height; i++)
	//	for (int j = 0; j < board_width; j++)
	//		obj.push_back(Point3f((float)j * square_size, (float)i * square_size, 0));

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
	
	if (found && numCaptures < num_imgs && timeForNewCheckerboard) {
		// Do a new capture
		numCaptures += 1;

		vector< Point3f > obj;
		for (int i = 0; i < board_height; i++)
			for (int j = 0; j < board_width; j++)
				obj.push_back(Point3f((float)j * square_size, (float)i * square_size, 0));

		image_points.push_back(corners);
		object_points.push_back(obj);

	}

	// We have enough samples -- finish calibrating.
	if (numCaptures == num_imgs) {
		int flag = 0;
		flag |= CV_CALIB_FIX_K4;
		flag |= CV_CALIB_FIX_K5;
		calibrateCamera(object_points, image_points, in.size(), K, D, rvecs, tvecs, flag);
		complete = true;
	}


}

bool CalibrationState::LoadFromFile(string path) {
	// Populates the model from a file.
	bool success = false;



	complete = success;
	return success;
}

bool CalibrationState::SaveToFile(string fileName) {
	return false;
}