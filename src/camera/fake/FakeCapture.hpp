#pragma once

#include <camera/CaptureBase.hpp>

#include <opencv2/opencv.hpp>
#include <atomic>

class FakeCapture : public CaptureBase {
public:
	FakeCapture();
	~FakeCapture() override;
	bool Init(const int deviceIndex) override;
	void Update() override;
	cv::Mat GetFrame() override;
	const std::string GetDeviceName() override;

	string dataPath;

private:
	cv::VideoCapture *cap;
	cv::Mat frame;

	bool waitIfEmpty = true;


	int frameCount = 0;

	bool playAsFastAsPossible = false;
	float timeSinceLastVideoFrame = 0.0;
	float lastVideoFrameTime = 0.0;
};