#include "PS4EyeCapture.hpp"

PS4EyeCapture::PS4EyeCapture()
{
	std::cout << "[PS4EyeCapture] Constructor called.\n";
}

PS4EyeCapture::~PS4EyeCapture()
{
	if (eye) {
		eye->set_led_off();
		eye->shutdown();
	}
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

		eye->firmware_path = ofToDataPath("firmware.bin", true).c_str();
		Kino::app_log.LogDebug("Loading at %s\n", eye->firmware_path.c_str());
		eye->firmware_upload();

		//mode 0: 60,30,15,8 fps 1280x800
		//mode 1: 120,60,30,15,8 fps 640x400
		//mode 2: 240,120,60,30 fps 320x192

		bool res = eye->init(0, 60);
		if (!res) {
			Kino::app_log.LogError("PS4EyeCapture failed to init device #%i.\n", deviceIndex);
			return false;
		}

		eye->start();

		width = eye->getWidth();
		height = eye->getHeight();

		frame_rgb_left = new uint8_t[width * height * 3];
		frame_rgb_right = new uint8_t[width * height * 3];
		memset(frame_rgb_left, 0, width * height * 3);
		memset(frame_rgb_right, 0, width * height * 3);

		return res;
	}
	return false;

	
	//this->deviceIndex = deviceIndex;

	//// list out the devices
	//std::vector<PS4EYECam::PS4EYERef> devices(PS4EYECam::getDevices());

	//if (deviceIndex >= 0 && deviceIndex < (int)devices.size()) {
	//	eye = devices[deviceIndex];
	//}
	//else {
	//	Kino::app_log.LogError("PS4EyeCapture failed to init device #%i (index out of bounds; only %i devices are attached).\n", deviceIndex, (int)devices.size());
	//	return false;
	//}

	//eye->firmware_path = ofToDataPath("firmware.bin", true).c_str();
	//Kino::app_log.LogDebug("Loading at %s\n", eye->firmware_path.c_str());
	//eye->firmware_upload();

	//bool res = eye->init(1, 60);

	//if (!res) {
	//	Kino::app_log.LogError("PS4EyeCapture failed to init device #%i.\n", deviceIndex);
	//	return false;
	//}

	//eye->start();

	//width = eye->getWidth();
	//height = eye->getHeight();
	//memset(frame_rgb_left, 0, width * height * 3);
	//memset(frame_rgb_right, 0, width * height * 3);

	//return true;
}


void PS4EyeCapture::Update()
{

	if (eye && !frameIsReady) {
		bool res = ps4eye::PS4EYECam::updateDevices();

		bool isNewFrame = eye->isNewFrame();
		if (isNewFrame) {
			eye->check_ff71();
			eyeframe *eyeFrame = eye->getLastVideoFramePointer();
			
			// Create YUYV mats from camera data
			cv::Mat leftYUYV(height, width, CV_8UC2, eyeFrame->videoLeftFrame);
			cv::Mat rightYUYV(height, width, CV_8UC2, eyeFrame->videoRightFrame);

			// Convert to BGR
			cvtColor(leftYUYV, leftFrame, COLOR_YUV2BGR_YUY2);
			cvtColor(rightYUYV, rightFrame, COLOR_YUV2BGR_YUY2);

			frameIsReady = true;
		}
	}
	
}

cv::Mat PS4EyeCapture::GetFrame()
{
	return leftFrame;
}


const std::string PS4EyeCapture::GetDeviceName() {
	return "PS4EYE";
}