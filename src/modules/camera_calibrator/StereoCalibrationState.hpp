#pragma once

#include "ofMain.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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
	bool FindCheckerboards(cv::InputArray inLeft, cv::InputArray inRight, vector<Point2f> &cornersLeft = vector<Point2f>(), vector<Point2f> &cornersRight = vector<Point2f>());
	bool QueueImages(cv::InputArray inLeft, cv::InputArray inRight);


	// A cached version of `cv::undistort()`. Significantly faster.
	void UndistortImage(cv::InputArray inLeft, cv::InputArray inRight, cv::OutputArray outLeft, cv::OutputArray outRight);

	// Loads `cameraMatrix` and `distortionCoeffs` from a file on disk (determined by unique_id).
	bool LoadFromFile();

	// Saves `cameraMatrix` and `distortionCoeffs` to a file on disk (determined by unique_id).
	bool SaveToFile();

	// Clears all information acquired from the calibration.
	void Reset();

	void CalibrateWithImageSet();

	bool calibrateIndividually = true;

	vector<vector<Point3f>> objectPoints;
	vector<vector<Point2f>> imagePointsLeft, imagePointsRight;

	// Variables needed to restore from disk
	Mat cameraMatrixLeft, cameraMatrixRight;
	Mat distortionCoeffsLeft, distortionCoeffsRight;
	Vec3d T;
	Mat R, F, E;

	// Rectification parameters
	cv::Mat R1, R2, P1, P2, Q;

	// http://www.hasper.info/opencv-draw-epipolar-lines/
	/**
	* \brief Compute and draw the epipolar lines in two images
	*      associated to each other by a fundamental matrix
	*
	* \param title     Title of the window to display
	* \param F         Fundamental matrix
	* \param img1      First image
	* \param img2      Second image
	* \param points1   Set of points in the first image
	* \param points2   Set of points in the second image matching to the first set
	* \param inlierDistance      Points with a high distance to the epipolar lines are
	*                not displayed. If it is negative, all points are displayed
	**/
	template <typename T1, typename T2>
	static cv::Mat drawEpipolarLines(const std::string& title, const cv::Matx<T1, 3, 3> F,
		const cv::Mat& img1, const cv::Mat& img2,
		const std::vector<cv::Point_<T2>> points1,
		const std::vector<cv::Point_<T2>> points2,
		const float inlierDistance = -1)
	{
		CV_Assert(img1.size() == img2.size() && img1.type() == img2.type());

		cv::Mat outImg(img1.rows, img1.cols * 2, CV_8UC3);
		cv::Rect rect1(0, 0, img1.cols, img1.rows);
		cv::Rect rect2(img1.cols, 0, img1.cols, img1.rows);
		/*
		* Allow color drawing
		*/
		if (img1.type() == CV_8U)
		{
			cv::cvtColor(img1, outImg(rect1), CV_GRAY2BGR);
			cv::cvtColor(img2, outImg(rect2), CV_GRAY2BGR);
		}
		else
		{
			img1.copyTo(outImg(rect1));
			img2.copyTo(outImg(rect2));
		}
		std::vector<cv::Vec<T2, 3>> epilines1, epilines2;
		if (!points1.empty() && !points2.empty()) {
			cv::computeCorrespondEpilines(points1, 1, F, epilines1); //Index starts with 1
			cv::computeCorrespondEpilines(points2, 2, F, epilines2);
		}

		CV_Assert(points1.size() == points2.size() &&
			points2.size() == epilines1.size() &&
			epilines1.size() == epilines2.size());

		cv::RNG rng(0);
		for (size_t i = 0; i<points1.size(); i++)
		{
			if (inlierDistance > 0)
			{
				if (distancePointLine(points1[i], epilines2[i]) > inlierDistance ||
					distancePointLine(points2[i], epilines1[i]) > inlierDistance)
				{
					//The point match is no inlier
					continue;
				}
			}
			/*
			* Epipolar lines of the 1st point set are drawn in the 2nd image and vice-versa
			*/
			cv::Scalar color(rng(256), rng(256), rng(256));

			cv::line(outImg(rect2),
				cv::Point(0, -epilines1[i][2] / epilines1[i][1]),
				cv::Point(img1.cols, -(epilines1[i][2] + epilines1[i][0] * img1.cols) / epilines1[i][1]),
				color);
			cv::circle(outImg(rect1), points1[i], 3, color, -1, CV_AA);

			cv::line(outImg(rect1),
				cv::Point(0, -epilines2[i][2] / epilines2[i][1]),
				cv::Point(img2.cols, -(epilines2[i][2] + epilines2[i][0] * img2.cols) / epilines2[i][1]),
				color);
			cv::circle(outImg(rect2), points2[i], 3, color, -1, CV_AA);
		}
		return outImg;
		//cv::imshow(title, outImg);
		//cv::waitKey(1);
	}

	template <typename T>
	static float distancePointLine(const cv::Point_<T> point, const cv::Vec<T, 3>& line)
	{
		//Line is given as a*x + b*y + c = 0
		return std::fabsf(line(0)*point.x + line(1)*point.y + line(2))
			/ std::sqrt(line(0)*line(0) + line(1)*line(1));
	}

private:
	

	bool mapsCreated = false;
	cv::Mat map1_left, map2_left, map1_right, map2_right;

	bool ValidateDir(ofDirectory dir);

};