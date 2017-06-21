#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "modules/ModuleCommon.hpp"
#include "gui/UsesGUI.hpp"
#include "config/ConfigHandler.hpp"

#include "ofxDarknet.h"

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include <helpers/Paths.hpp>

class ClassifierLens : public ModuleCommon, public UsesGUI {
public:
	ClassifierLens();
	~ClassifierLens();

	void ProcessFrame(cv::InputArray in, cv::OutputArray out);
	void DrawGUI() override;

	void InitFromConfig();

	bool doDownsampling = false;
	float downSampleRatio = 0.5f;

	struct Size {
		int x;
		int y;
	};

private:
	std::vector< detected_object > detections;
	ofxDarknet darknet;

	float threshold = 0.25;

	Size lastImageSize{ 0, 0 };

	bool initialized = false;

	string cfg_file;
	string weights_file;
	string names_list;

};