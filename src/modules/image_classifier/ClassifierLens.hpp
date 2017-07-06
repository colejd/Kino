#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/tracking.hpp>

#include "modules/ModuleCommon.hpp"
#include "gui/UsesGUI.hpp"
#include "config/ConfigHandler.hpp"

#include "ofxDarknet.h"

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include <helpers/Paths.hpp>

using namespace std;
using namespace cv;

class ClassifierLens : public ModuleCommon, public UsesGUI {
public:
	ClassifierLens();
	~ClassifierLens();

	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight);
	void DrawGUI() override;

	void InitFromConfig();

private:
	ofxDarknet darknet;
	std::vector< detected_object > detectionsLeft;
	std::vector< detected_object > detectionsRight;

	float threshold = 0.25;

	cv::Size lastSizeLeft;
	cv::Size lastSizeRight;

	bool initialized = false;

	string cfg_file;
	string weights_file;
	string names_list;

	// These are smart pointers, no need to manually allocate.
	Ptr<Tracker> trackerLeft;
	Ptr<Tracker> trackerRight;

	bool doDownsampling = false;
	float downSampleRatio = 0.5f;

	void DrawDetections(InputOutputArray mat, std::vector< detected_object > detections);

};