#pragma once

#include "ofMain.h"
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

/**
Manages the calibration and undistortion of a single camera feed. To use, iteratively call
`ProcessImage()` until it's done, then use `UndistortImage()` to undistort any image from
that camera.
*/
class CalibrationState {
public:
	bool hasCapture = false;
	bool complete = false;

	// Number of captures acquired
	int numCaptures = 0;
	// Number of captures needed to finalize calibration
	int capturesRequired;

	// Number of squares (width x height) on the checkerboard.
	Size board_size;
	// Size of each checkerboard square in millimeters (arbitrary, you can change units to whatever you want)
	float square_size;

	// The amount of error calculated from the reprojection. Should be as close to 0 as possible.
	double reprojectionError = 0.0;



	CalibrationState(int board_width = 9, int board_height = 6, int capturesRequired = 20, float square_size = 26.0);

	// Runs the calibration routine on `in`. Any found checkerboards are drawn to `out`.
	void ProcessImage(cv::InputArray in, cv::InputOutputArray out);

	// A cached version of `cv::undistort()`. Significantly faster.
	void UndistortImage(cv::InputArray in, cv::OutputArray out);

	// Loads `cameraMatrix` and `distortionCoeffs` from a file on disk.
	bool LoadFromFile(string path);

	// Saves `cameraMatrix` and `distortionCoeffs` to a file on disk.
	bool SaveToFile(string fileName);

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

	Mat cameraMatrix; // Intrinsics
	Mat distortionCoeffs;

	// Rotation / translation vectors (derived by calibrateCamera)
	vector<Mat> rvecs, tvecs;

	bool mapsCreated = false;
	cv::Mat map1, map2;

};