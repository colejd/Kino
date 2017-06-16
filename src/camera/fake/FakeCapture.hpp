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
	const bool FrameIsReady() override;
	cv::Mat GetFrame() override;

	string dataPath;

private:
	cv::VideoCapture *cap;
	cv::Mat frame;

	bool waitIfEmpty = false;
	std::atomic<bool> frameIsReady = false;


	int frameCount = 0;
};