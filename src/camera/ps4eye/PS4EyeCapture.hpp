#pragma once

#include "ofMain.h"

#include <opencv2/opencv.hpp>
#include <atomic>

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include "camera/CaptureBase.hpp"

#include <camera/ps4eye/driver/ps4eye.h>

using namespace cv;
using namespace ps4eye;

class PS4EyeCapture : public CaptureBase {
public:
	PS4EyeCapture();
	~PS4EyeCapture() override;
	bool Init(const int deviceIndex) override;
	void Update() override;
	cv::Mat GetFrame() override;
	const std::string GetDeviceName() override;

private:
	cv::Mat leftFrame;
	cv::Mat rightFrame;

	bool waitIfEmpty = true;

	PS4EYECam::PS4EYERef eye;

	uint8_t *frame_rgb_left;
	uint8_t *frame_rgb_right;


};

