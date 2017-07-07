#pragma once

#include <opencv2/opencv.hpp>
#include <atomic>

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include "camera/CaptureBase.hpp"

#include "driver/ps3eye.h"

class PS3EyeCapture : public CaptureBase {
public:
	PS3EyeCapture();
	~PS3EyeCapture() override;
	bool Init(const int deviceIndex) override;
	void Update() override;
	cv::Mat GetFrame() override;
	const std::string GetDeviceName() override;

private:
	cv::Mat frame;

	bool waitIfEmpty = true;

	ps3eye::PS3EYECam::PS3EYERef eye;
	uint8_t *convertedData;

};