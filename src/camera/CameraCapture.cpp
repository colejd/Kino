#include "CameraCapture.hpp"

CameraCapture::CameraCapture()
{
	std::cout << "[CameraCapture] Constructor called\n";
}

CameraCapture::~CameraCapture()
{
	std::cout << "[CameraCapture] Destructor called\n";
	//Stop the thread if it's running
	if (initialized) {
		StopCapturing();
	}

}

bool CameraCapture::StartFakeCapture(string videoPath, bool threaded) {
	captureMethod = new FakeCapture();
	dynamic_cast<FakeCapture*>(captureMethod)->dataPath = videoPath;

	initialized = captureMethod->Init(0);
	if (initialized == false) {
		std::cout << "[CameraCapture] StartFakeCapture failed - capture device did not init correctly\n";
		return false;
	}

	this->threaded = threaded;
	if (threaded) {
		StartThreadedUpdate();
	}

	return true;
}

bool CameraCapture::StartCapturing(int index, CAPTURE_TYPE type, bool threaded)
{
	if (type == CAPTURE_TYPE::GENERIC) {
		captureMethod = new SystemCameraCapture();
	}
	else if (type == CAPTURE_TYPE::PS3EYE) {
		captureMethod = new PS3EyeCapture();
	}
	else {
		return false;
	}

	initialized = captureMethod->Init(index);
	if (initialized == false) {
		std::cout << "[CameraCapture] StartCapturing failed - capture device did not init correctly\n";
		return false;
	}

	this->threaded = threaded;
	if (threaded) {
		StartThreadedUpdate();
	}

	return true;
}

bool CameraCapture::StopCapturing()
{
	if (threaded) StopThreadedUpdate();
	if (captureMethod != nullptr) {
		delete captureMethod;
	}
	initialized = false;

	return true;
}

//Update latestFrame with the newest data from the capture.
void CameraCapture::UpdateCapture()
{
	//std::cout << "Update (threaded: " << threaded << ")\n";
	captureMethod->Update();
	captureMethod->GetFrame().copyTo(latestFrame);
}

//Return the latest frame.
cv::Mat CameraCapture::RetrieveCapture()
{
	if (threaded) std::lock_guard<std::mutex> lock(frameMutex); //Mutex lock
	return latestFrame;
}

bool CameraCapture::FrameIsReady()
{
	//return captureMethod->FrameIsReady();
	//Leave as just true for now; it's faster and turning it on
	//more or less does nothing anyway
	return true;
}

const bool CameraCapture::IsThreaded()
{
	return threaded;
}

const bool CameraCapture::IsInitialized()
{
	return initialized;
}

void CameraCapture::StartThreadedUpdate()
{
	std::cout << "[CameraCapture] Starting threaded update.\n";
	captureThread = std::thread(&CameraCapture::ThreadedUpdateCapture, this);
}

void CameraCapture::StopThreadedUpdate()
{
	threadShouldStop = true;
	captureThread.join();
	std::cout << "[CameraCapture] Threaded update joined\n";
}

void CameraCapture::ThreadedUpdateCapture()
{
	std::cout << "[CameraCapture] Began threaded update loop.\n";
	while (threadShouldStop == false) {
		if (threaded) std::lock_guard<std::mutex> lock(frameMutex); //Mutex lock
		UpdateCapture();
	}
	std::cout << "[CameraCapture] Reached end of threaded update loop.\n";
}
