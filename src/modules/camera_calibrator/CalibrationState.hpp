#pragma once

#include "ofMain.h"
#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

class CalibrationState {
public:

	bool complete = false;

	int numBoards = 20;
	float secondsBetweenCaptures = 1.0;
	float lastFrameTime = 0.0;
	float secondsSinceLastCapture = 0.0;
	int numCaptures = 0;

	Size board_size;
	int board_width;
	int board_height;
	int board_n;
	int num_imgs;
	float square_size; // Size of the square in millimeters (arbitrary, you can change this to whatever you want)

	vector< vector< Point3f > > object_points;
	vector< vector< Point2f > > image_points;
	vector< Point2f > corners;

	Mat K; // intrinsic
	Mat D; // distCoeffs


	// Reference points

	// In-loop var storage
	//bool found = false;



	CalibrationState(int board_width = 9, int board_height = 6, int num_imgs = 20, float square_size = 26.0);

	void ProcessImage(cv::InputArray in, cv::InputOutputArray out);

	bool LoadFromFile(string path);

	bool SaveToFile(string fileName);

private:
	vector< Mat > rvecs, tvecs;

};