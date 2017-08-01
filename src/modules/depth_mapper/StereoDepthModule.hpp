#pragma once

#include "ofMain.h"

#include <opencv2/opencv.hpp>
#include <opencv2/ximgproc/disparity_filter.hpp>

#include <ofxTimeMeasurements.h>

#include <modules/ModuleCommon.hpp>
#include <KinoGlobals.hpp>

#include <gl/GL.h>
#include <gl/glew.h>

using namespace std;
using namespace cv;
using namespace cv::ximgproc;

class StereoDepthModule : public ModuleCommon {
public:
	StereoDepthModule();
	~StereoDepthModule();

	std::string GetName() override {
		return "Stereo Depth";
	}

	// Runs calibration or undistortion on `in` and writes the result to `out`. ID is expected to be "LEFT" or "RIGHT".
	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) override;
	void DrawGUI() override;

	Ptr<StereoBM> leftBM;
	Ptr<StereoMatcher> rightBM;


private:
	bool moduleCanRun = false;

	int numDisparities = 112; // 112 (160)
	int blockSize = 9; // 9 (15)

	cv::Mat disparity;

	bool doDownsampling = false;
	float downSampleRatio = 0.5f;

	bool filter = false;
	Ptr<DisparityWLSFilter> wls_filter;
	float wls_lambda = 8000.0;
	float wls_sigma = 1.5;

	void DrawImguiMat(InputArray mat, string id);

	// Keeps track of GL texture assignments, using unique ID strings as keys.
	std::map<string, GLuint> textureMap;

};