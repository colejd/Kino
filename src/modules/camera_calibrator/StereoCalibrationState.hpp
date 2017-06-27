#pragma once

#include "ofMain.h"
#include <opencv2/opencv.hpp>
#include <string>

#include <KinoGlobals.hpp>

using namespace cv;
using namespace std;

// http://sourishghosh.com/2016/stereo-calibration-cpp-opencv/

class StereoCalibrationState {
public:
	bool hasCapture = false;
	bool complete = false;

	string unique_id;

	// Number of captures acquired
	int numCaptures = 0;
	// Number of captures needed to finalize calibration
	int capturesRequired;

	// Number of squares (width x height) on the checkerboard.
	Size boardSize;
	// Size of each checkerboard square in millimeters (arbitrary, you can change units to whatever you want)
	float squareSize;

	// The amount of error calculated from the reprojection. Should be as close to 0 as possible.
	double reprojectionError = 0.0;



	StereoCalibrationState(int board_width = 9, int board_height = 6, int capturesRequired = 20, float squareSize = 26.0);

	bool QueueImages(cv::InputArray inLeft, cv::InputArray inRight);


	// A cached version of `cv::undistort()`. Significantly faster.
	void UndistortImage(cv::InputArray in, cv::OutputArray out, string id);

	// Loads `cameraMatrix` and `distortionCoeffs` from a file on disk (determined by unique_id).
	bool LoadFromFile();

	// Saves `cameraMatrix` and `distortionCoeffs` to a file on disk (determined by unique_id).
	bool SaveToFile();

	// Clears all information acquired from the calibration.
	void Reset();

	void CalibrateWithImageSet();


private:

	vector<vector<Point3f>> objectPoints;
	//vector<vector<Point2f>> imagePoints1, imagePoints2;
	vector<vector<Point2f>> imagePointsLeft, imagePointsRight;
	vector<Point2f> cornersLeft, cornersRight;

	// Variables needed to restore from disk
	Mat cameraMatrixLeft, cameraMatrixRight;
	Mat distortionCoeffsLeft, distortionCoeffsRight;
	Vec3d T;
	Mat R, F, E;

	// Rectification parameters
	cv::Mat R1, R2, P1, P2, Q;
	

	bool mapsCreated = false;
	cv::Mat map1_left, map2_left, map1_right, map2_right;

	bool ValidateDir(ofDirectory dir);

};