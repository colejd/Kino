#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "modules/ModuleCommon.hpp"
#include "gui/UsesGUI.hpp"

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

	struct YoloConfig {
		string cfgFile;
		string weightFile;
		string namesList;
	};

	void InitWithConfig(YoloConfig config);

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

	const YoloConfig yolo = { "cfg/yolo.cfg", "yolo.weights", "cfg/coco.names" };
	const YoloConfig tinyYolo = { "cfg/tiny-yolo.cfg", "tiny-yolo.weights", "cfg/coco.names" };
	const YoloConfig yoloVoc = { "cfg/yolo-voc.cfg", "yolo-voc.weights", "cfg/voc.names" };
	const YoloConfig tinyYoloVoc = { "cfg/tiny-yolo-voc.cfg", "tiny-yolo-voc.weights", "cfg/voc.names" };
	const YoloConfig yolo9k = { "cfg/yolo9000.cfg", "yolo9000.weights", "cfg/9k.names" };

	ofImage nightmare;

	YoloConfig config;

	Size lastImageSize { 0, 0 };

};