#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"

#include "ofMain.h"

#include "modules/ModuleCommon.hpp"
#include "ofxTimeMeasurements.h"

using namespace std;
using namespace cv;

class FaceDetectorModule : public ModuleCommon {
public:
	FaceDetectorModule();
	~FaceDetectorModule();

	std::string GetName() override {
		return "Face Detector";
	}

	void ProcessFrame(cv::InputArray in, cv::OutputArray out);
	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) override;

	void DrawGUI() override;

private:
	cv::CascadeClassifier face_cascade;
	cv::CascadeClassifier eyes_cascade;

	float imageScale = 0.125;

};
