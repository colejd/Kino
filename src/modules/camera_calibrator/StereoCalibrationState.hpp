#pragma once


#include "ofMain.h"
#include <opencv2/opencv.hpp>
#include <string>

#include <KinoGlobals.hpp>

using namespace cv;
using namespace std;

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

	// Runs the calibration routine on `in`. Any found checkerboards are drawn to `out`.
	void ProcessImages(cv::InputArray inLeft, cv::InputArray inRight, cv::InputOutputArray outLeft, cv::InputOutputArray outRight);

	// A cached version of `cv::undistort()`. Significantly faster.
	void UndistortImage(cv::InputArray in, cv::OutputArray out, string id);

	// Loads `cameraMatrix` and `distortionCoeffs` from a file on disk (determined by unique_id).
	bool LoadFromFile();

	// Saves `cameraMatrix` and `distortionCoeffs` to a file on disk (determined by unique_id).
	bool SaveToFile();

	// Clears all information acquired from the calibration.
	void Reset();


private:

	// Timing information
	float secondsBetweenCaptures = 1.0;
	float lastFrameTime = 0.0;
	float secondsSinceLastCapture = 0.0;

	vector<vector<Point3f>> objectPoints;
	vector<vector<Point2f>> imagePoints;
	vector<Point2f> corners;

	vector<float> reprojErrs;

	Mat cameraMatrix = Mat::eye(3, 3, CV_64F);
	Mat distortionCoeffs = Mat::zeros(8, 1, CV_64F);

	// Rotation / translation vectors (derived by calibrateCamera)
	vector<Mat> rvecs, tvecs;

	bool mapsCreated = false;
	cv::Mat map1, map2;

};