#pragma once

#include "ofMain.h"
#include <opencv2/opencv.hpp>
#include <string>

#include <KinoGlobals.hpp>

using namespace cv;
using namespace std;

/**
Manages the calibration and undistortion of a single camera feed. To use, iteratively call
`IngestImageForCalibration()` until it's done, then use `UndistortImage()` to undistort any image from
that camera.

Most of the calibration code itself was adapted to a per-frame technique from
https://github.com/opencv/opencv/blob/master/samples/cpp/tutorial_code/calib3d/camera_calibration/camera_calibration.cpp

*/
class CalibrationState {
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



	CalibrationState(int board_width = 9, int board_height = 6, int capturesRequired = 20, float squareSize = 26.0);

	// Runs the calibration routine on `in`. Any found checkerboards are drawn to `out`.
	void IngestImageForCalibration(cv::InputArray in, cv::InputOutputArray out);

	bool QueueImage(cv::InputArray in, cv::InputOutputArray out = cv::Mat(), bool keepResults = true);


	// A cached version of `cv::undistort()`. Significantly faster.
	void UndistortImage(cv::InputArray in, cv::OutputArray out);

	// Loads `cameraMatrix` and `distortionCoeffs` from a file on disk (determined by unique_id).
	bool LoadFromFile();

	// Saves `cameraMatrix` and `distortionCoeffs` to a file on disk (determined by unique_id).
	bool SaveToFile();

	// Clears all information acquired from the calibration.
	void Reset();

	void CalibrateWithImageSet();


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