#pragma once

#include <opencv2/opencv.hpp>
#include <atomic>

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"
#include "config/ConfigHandler.hpp"

#include "camera/CaptureBase.hpp"

class SystemCameraCapture : public CaptureBase {
public:
	SystemCameraCapture();
	~SystemCameraCapture() override;
	bool Init(const int deviceIndex) override;
	void Update() override;
	cv::Mat GetFrame() override;
	const std::string GetDeviceName() override;

private:
	cv::VideoCapture *cap;
	cv::Mat frame;

	bool waitIfEmpty = true;

};