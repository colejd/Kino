#include "FakeCapture.hpp"

using namespace cv;

FakeCapture::FakeCapture()
{
	std::cout << "[FakeCapture] Constructor called.\n";
	cap = new VideoCapture();
}

FakeCapture::~FakeCapture()
{
	std::cout << "[FakeCapture] Destructor called.\n";
	delete cap;
}

bool FakeCapture::Init(const int deviceIndex)
{
	this->deviceIndex = deviceIndex;
	bool success = cap->open(dataPath);
	if (success) {
		//THESE LINES ARE NEEDED FOR SOME REASON OR THE ISIGHT CAMERA WON'T WORK
		//cap->set(CV_CAP_PROP_FRAME_WIDTH, 480); //KEEP ME (500)
		//cap->set(CV_CAP_PROP_FRAME_HEIGHT, 640); //KEEP ME (600)
		return true;
	}
	return false;
}

void FakeCapture::Update()
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
			cv::resize(frame, frame, cv::Size(), 0.5, 0.5, INTER_NEAREST);
			//cv::imshow("wow", frame);
			frameIsReady = true;

			frameCount += 1;
			// If we've reached the last frame, reset.
			if (frameCount == cap->get(CV_CAP_PROP_FRAME_COUNT)) {
				cap->set(CV_CAP_PROP_POS_FRAMES, 0);
				frameCount = 0;
			}
		}
	}

}

const bool FakeCapture::FrameIsReady()
{
	return frameIsReady;
}

cv::Mat FakeCapture::GetFrame()
{
	return frame;
}
