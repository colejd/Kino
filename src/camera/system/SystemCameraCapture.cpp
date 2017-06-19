#include "SystemCameraCapture.hpp"

using namespace cv;

SystemCameraCapture::SystemCameraCapture()
{
	std::cout << "[SystemCameraCapture] Constructor called.\n";
	cap = new VideoCapture();
}

SystemCameraCapture::~SystemCameraCapture()
{
	std::cout << "[SystemCameraCapture] Destructor called.\n";
	delete cap;
}

bool SystemCameraCapture::Init(const int deviceIndex)
{
	this->deviceIndex = deviceIndex;
	bool success = cap->open(deviceIndex);
	if (success) {
		//THESE LINES ARE NEEDED FOR SOME REASON OR THE ISIGHT CAMERA WON'T WORK
		cap->set(CV_CAP_PROP_FRAME_WIDTH, ConfigHandler::GetValue("PREFERRED_CAMERA_RES_X", 640).asInt()); // 480
		cap->set(CV_CAP_PROP_FRAME_HEIGHT, ConfigHandler::GetValue("PREFERRED_CAMERA_RES_Y", 480).asInt()); // 640
		return true;
	}
	return false;
}

void SystemCameraCapture::Update()
{
	frameIsReady = false;
	if (cap->isOpened()) {
		if (waitIfEmpty && frame.empty() && false) { //&&false does some stuff here 
			//std::cout << "It's empty\n";
			//Wait until the frame gets filled by actual camera data.
			int maxWaitIterations = 500;
			int waitIterations = 0;
			while (waitIterations < maxWaitIterations) {
				cap->read(frame);
				waitIterations += 1;
				//std::cout << "Waiting\n";
				if (frame.empty() == false) break;
			}
			//std::cout << "Now it's not\n";
		}
		//Otherwise just read from cap into the frame.
		else {
			cap->read(frame);
			//cv::imshow("wow", frame);
			frameIsReady = true;
		}
	}

}

const bool SystemCameraCapture::FrameIsReady()
{
	return frameIsReady;
}

cv::Mat SystemCameraCapture::GetFrame()
{
	return frame;
}
