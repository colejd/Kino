#include "PS4EyeCapture.hpp"

PS4EyeCapture::PS4EyeCapture()
{
	std::cout << "[PS4EyeCapture] Constructor called.\n";
}

PS4EyeCapture::~PS4EyeCapture()
{
	if (eye) eye->stop();
	if (frame_rgb_left != nullptr) free(frame_rgb_left);
	if (frame_rgb_right != nullptr) free(frame_rgb_right);
	std::cout << "[PS4EyeCapture] Destructor called.\n";
}

bool PS4EyeCapture::Init(const int deviceIndex)
{
	this->deviceIndex = deviceIndex;

	// list out the devices
	std::vector<PS4EYECam::PS4EYERef> devices(PS4EYECam::getDevices());

	if (devices.size() && devices.size() > deviceIndex)
	{
		eye = devices.at(deviceIndex);

		eye->firmware_path = ofToDataPath("firmware.bin").c_str();
		eye->firmware_upload();

		//mode 0: 60,30,15,8 fps 1280x800
		//mode 1: 120,60,30,15,8 fps 640x400
		//mode 2: 240,120,60,30 fps 320x192

		bool res = eye->init(0, 60);
		if (!res) {
			Kino::app_log.LogError("PS4EyeCapture failed to init device #%i.\n", deviceIndex);
		}

		width = eye->getWidth();
		height = eye->getHeight();
		eye->start();

		memset(frame_rgb_left, 0, width * height * 3);
		memset(frame_rgb_right, 0, width * height * 3);

		return res;
	}
	return false;
}


void PS4EyeCapture::Update()
{
	frameIsReady = false;
	eyeframe *eyeFrame;

	if (eye) {

		bool isNewFrame = eye->isNewFrame();
		if (isNewFrame) {
			eye->check_ff71();
			eyeFrame = eye->getLastVideoFramePointer();

			if (eye->rightflag) {
				leftFrame = cv::Mat(height, width, CV_8UC3, eyeFrame->videoLeftFrame);
				rightFrame = cv::Mat(height, width, CV_8UC3, eyeFrame->videoRightFrame);

				frameIsReady = true;
			}

		}
	}
}

const bool PS4EyeCapture::FrameIsReady()
{
	return frameIsReady;
}

cv::Mat PS4EyeCapture::GetFrame()
{
	return leftFrame;
}


const std::string PS4EyeCapture::GetDeviceName() {
	return "PS4EYE";
}