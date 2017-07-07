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
	if (cap->isOpened() && !frameIsReady) {

		if (!playAsFastAsPossible) {
			// Wait until enough time has elapsed to constitute a frame in the video.
			double fps = cap->get(CAP_PROP_FPS);
			float time = ofGetElapsedTimef();
			timeSinceLastVideoFrame += time - lastVideoFrameTime;
			lastVideoFrameTime = time;
			if (timeSinceLastVideoFrame < (1.0 / fps)) {
				return;
			}
			timeSinceLastVideoFrame = 0.0;
		}

		cap->read(frame);
		cv::resize(frame, frame, cv::Size(), 0.5, 0.5, INTER_NEAREST);
		frameIsReady = true;

		frameCount += 1;
		// If we've reached the last frame, reset.
		if (frameCount == cap->get(CV_CAP_PROP_FRAME_COUNT)) {
			cap->set(CV_CAP_PROP_POS_FRAMES, 0);
			frameCount = 0;
		}
	}

}

cv::Mat FakeCapture::GetFrame()
{
	return frame;
}

const std::string FakeCapture::GetDeviceName() {
	return "FAKE";
}