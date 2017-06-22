#include "PS3EyeCapture.hpp"

using namespace cv;
using namespace ps3eye;

PS3EyeCapture::PS3EyeCapture()
{
	std::cout << "[PS3EyeCapture] Constructor called.\n";
}

PS3EyeCapture::~PS3EyeCapture()
{
	if (eye) eye->stop();
	if (convertedData != nullptr) free(convertedData);
	std::cout << "[PS3EyeCapture] Destructor called.\n";
}

bool PS3EyeCapture::Init(const int deviceIndex)
{
	this->deviceIndex = deviceIndex;

	// list out the devices
	std::vector<PS3EYECam::PS3EYERef> devices(PS3EYECam::getDevices());

	if (devices.size() && devices.size() > deviceIndex)
	{
		eye = devices.at(deviceIndex);
		bool res = eye->init(640, 480, 60, PS3EYECam::EOutputFormat::BGR);
		width = eye->getWidth();
		height = eye->getHeight();
		eye->start();
		eye->setAutogain(true);
		//eye->setAutoWhiteBalance(true);
		convertedData = (unsigned char*)malloc(width * height * 3 * sizeof(unsigned char));
		return res;
	}
	return false;
}

void PS3EyeCapture::Update()
{
	frameIsReady = false;
	if (eye) {
		eye->getFrame(convertedData);

		//cv::Mat tempFrame = cv::Mat(height, width, CV_8UC1, convertedData);
		//cvtColor(tempFrame, frame, COLOR_BayerGB2BGR);

		frame = cv::Mat(height, width, CV_8UC3, convertedData);

		frameIsReady = true;
	}
}

const bool PS3EyeCapture::FrameIsReady()
{
	return frameIsReady;
}

cv::Mat PS3EyeCapture::GetFrame()
{
	return frame;
}


const std::string PS3EyeCapture::GetDeviceName() {
	return "PS3EYE";
}