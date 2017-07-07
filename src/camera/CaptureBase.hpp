#pragma once

#include <opencv2/opencv.hpp>

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"
#include <string>
#include <atomic>

/**
Abstract class defining common members that any deriving class will need to be
used in a CameraCapture implementation.
*/
class CaptureBase {
public:
	virtual ~CaptureBase() {};
	virtual bool Init(const int deviceIndex) = 0;
	virtual void Update() = 0;
	// Tells the capture to grab a new frame from the device, but not unless the last frame has been consumed.
	virtual cv::Mat GetFrame() = 0;
	virtual const std::string GetDeviceName() = 0;

	void ConsumeCapture() {
		frameIsReady = false;
	}

	const bool FrameIsReady() {
		return frameIsReady;
	}

protected:
	int width;
	int height;

	int deviceIndex;
	std::atomic<bool> frameIsReady = false;

};