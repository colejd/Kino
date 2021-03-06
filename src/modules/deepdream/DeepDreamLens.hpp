#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "modules/ModuleCommon.hpp"
#include "config/ConfigHandler.hpp"

#include "ofMain.h"
#include "ofxDarknet.h"

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include <helpers/Paths.hpp>

using namespace std;
using namespace cv;

class DeepDreamLens : public ModuleCommon {
public:
	DeepDreamLens();
	~DeepDreamLens();

	std::string GetName() override {
		return "DeepDream";
	}

	void ProcessFrame(cv::InputArray in, cv::OutputArray out);
	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) override;
	void DrawGUI() override;

	void Initialize();

private:
	ofxDarknet darknet;
	bool initialized = false;
	ofImage nightmare;

	// Deepdream variables
	int max_layer = 13;
	int iters = 1;
	int octaves = 2;
	float thresh = 0.85;
	int range = 3;
	int norm = 1;
	float rate = 0.01;
	float blendAmt = 0.5;
	int rounds = 1;

};