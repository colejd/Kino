#pragma once

#include "ofMain.h"

#include <opencv2/opencv.hpp>
#include <ofxTimeMeasurements.h>

#include <modules/ModuleCommon.hpp>
#include <gui/UsesGUI.hpp>
#include <KinoGlobals.hpp>

#include <gl/GL.h>
#include <gl/glew.h>

using namespace std;
using namespace cv;


class StereoDepthModule : public ModuleCommon, public UsesGUI {
public:
	StereoDepthModule();
	~StereoDepthModule();

	// Runs calibration or undistortion on `in` and writes the result to `out`. ID is expected to be "LEFT" or "RIGHT".
	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight);
	void DrawGUI() override;

	Ptr<StereoBM> sbm;


private:
	bool moduleCanRun = false;

	int numDisparities = 16;
	int blockSize = 9;

	cv::Mat disparity;

	GLuint previewTextureID;

};