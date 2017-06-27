#pragma once

#include "ofMain.h"

#include <opencv2/opencv.hpp>
#include <ofxTimeMeasurements.h>

#include <modules/ModuleCommon.hpp>
#include <gui/UsesGUI.hpp>
#include <KinoGlobals.hpp>

using namespace std;
using namespace cv;


class StereoDepthModule : public ModuleCommon, public UsesGUI {
public:
	StereoDepthModule();
	~StereoDepthModule();

	// Runs calibration or undistortion on `in` and writes the result to `out`. ID is expected to be "LEFT" or "RIGHT".
	void ProcessFrame(cv::InputArray in, cv::InputOutputArray out, string id);
	void DrawGUI() override;

	Ptr<StereoBM> sbm;


private:
	cv::Mat lastLeftMat;
	cv::Mat lastRightMat;

};