#pragma once

#include <atomic>
#include <mutex>

#include <opencv2/opencv.hpp>

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include "CaptureBase.hpp";
#include "system/SystemCameraCapture.hpp";
#include "ps3eye/PS3EyeCapture.hpp";
#include "fake/FakeCapture.hpp";

class CameraCapture {
public:

	/**
	Initialize what you need for the capture here
	*/
	CameraCapture();
	~CameraCapture();

	/**
	The type of camera device looked for by this object.
	*/ 
	enum class CAPTURE_TYPE {
		/** OS-recognized camera */
		GENERIC,
		/** Uses PS3EyeDriver */
		PS3EYE
	};

	/** 
	Begins the capture process.
	*/
	bool StartCapturing(int index, CAPTURE_TYPE type, bool threaded);

	/** 
	Starts a fake capture process. videoPath is relative to the data folder. 
	*/
	bool StartFakeCapture(string videoPath, bool threaded);

	/** 
	Stops the capture process.
	*/
	bool StopCapturing();

	void UpdateCapture();
	cv::Mat RetrieveCapture();

	bool FrameIsReady();

	//Getters
	const bool IsThreaded();
	const bool IsInitialized();

	int lastAttemptedDeviceIndex;
	CAPTURE_TYPE lastAttemptedDeviceType;
	bool lastAttemptedThreaded;

private:

	CaptureBase *captureMethod;

	bool initialized = false;
	bool threaded = false;
	std::thread captureThread;

	void StartThreadedUpdate();
	void StopThreadedUpdate();

	void ThreadedUpdateCapture();
	std::atomic<bool> threadShouldStop = false;

	//"if you have to pass a mat image between threads you have to use a mutex and call .CopyTo() to force a copy instead of a reference."
	std::mutex frameMutex;
	cv::Mat latestFrame;

};