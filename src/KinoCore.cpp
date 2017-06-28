#include "KinoCore.hpp"

using namespace cv;

KinoCore::KinoCore() {
	Setup();
}

KinoCore::~KinoCore() {
}

void KinoCore::Setup() {
	cv::ocl::setUseOpenCL(ConfigHandler::GetValue("OPENCV.USE_OPENCL", false).asBool());
	bool demoMode = ConfigHandler::GetValue("DEMO_SETTINGS.ACTIVE", false).asBool();

	PrintCVDebugInfo();

	// Initialize modules
	edgeDetector = EdgeDetectorModule();
	faceDetector = FaceDetectorModule();
	depthModule = StereoDepthModule();

	// Initialize captures
	capture1 = make_unique<CameraCapture>();
	capture2 = make_unique<CameraCapture>();

	if (demoMode) {
		// Demo mode: show only input from one capture source in fullscreen
		string cameraType = ConfigHandler::GetValue("DEMO_SETTINGS.CAMERA_MODE", "").asString();
		int cameraIndex = ConfigHandler::GetValue("DEMO_SETTINGS.CAMERA_INDEX", 0).asInt();

		Kino::app_log.AddLog("Demo Mode active (mode: %s)\n", cameraType.c_str());

		if (cameraType == "SYSTEM") {
			capture1->StartCapturing(cameraIndex, CameraCapture::CAPTURE_TYPE::GENERIC, true);
		}
		else if (cameraType == "PS3EYE") {
			capture1->StartCapturing(0, CameraCapture::CAPTURE_TYPE::PS3EYE, true);
		}
		else if (cameraType == "FAKE") {
			string fakeVideoPath = ConfigHandler::GetValue("DEMO_SETTINGS.FAKE_VIDEO_PATH", "").asString();
			capture1->StartFakeCapture(ofToDataPath(fakeVideoPath), true);
		}
		else {
			Kino::app_log.AddLog("ERROR: the camera type given (%s) does not match any accepted types. \
									Please use SYSTEM, PS3EYE, or FAKE.");
		}

	}
	else {
		// Normal stereo capture session
		capture1->StartCapturing(0, CameraCapture::CAPTURE_TYPE::PS3EYE, true);
		capture2->StartCapturing(1, CameraCapture::CAPTURE_TYPE::PS3EYE, true);
	}

	// Init framebuffer
	framebuffer["LEFT"] = Frame();
	framebuffer["RIGHT"] = Frame();

	cameraCalibrator.RegisterCameras(capture1, capture2);

}

void KinoCore::Update() {

	// Update captures into framebuffer, swapping if desired by user settings
	ProcessCapture(capture1, !swapSides ? "LEFT" : "RIGHT");
	ProcessCapture(capture2, !swapSides ? "RIGHT" : "LEFT");

	// Run the stereo modules with the frame data
	ConsumeFrames();
}

/**
Fills the frame buffers when each camera is ready. Marks them as ready for synchronization purposes.
*/
void KinoCore::ProcessCapture(std::unique_ptr<CameraCapture> const& cap, string id) {
	TS_SCOPE("Process Capture");
	if (!cap->IsInitialized()) {
		framebuffer[id].MarkReady();
		return;
	}


	if (cap->IsThreaded() == false) {
		//If the capture isn't threaded, we need to manually update it before querying it here.
		TS_SCOPE("Update Capture");
		cap->UpdateCapture();
	}
	if (cap->FrameIsReady()) {

		if (!pauseCaptureUpdates) {
			TS_SCOPE("Frame Grab");
			//rawFrame = cap->RetrieveCapture().clone();
			cap->RetrieveCapture().copyTo(framebuffer[id].data);
			framebuffer[id].MarkReady();
		}

	}

}

/**
Synchronized frame consumption (runs only when both frames are ready).
Processes 
*/
void KinoCore::ConsumeFrames() {
	Frame *left = &framebuffer["LEFT"];
	Frame *right = &framebuffer["RIGHT"];

	// If either frame isn't ready, don't continue
	if (!left->IsReady() || !right->IsReady()) {
		return;
	}

	TS_SCOPE("Consume Frames");

	// Make copies of the frame data for processing
	TS_START_NIF("Frame In");
	cv::Mat leftTemp, rightTemp;
	left->data.copyTo(leftTemp);
	right->data.copyTo(rightTemp);
	TS_STOP_NIF("Frame In");

	// Process the data through each module
	TS_START_NIF("Frame Process");
	cameraCalibrator.ProcessFrames(leftTemp, rightTemp, leftTemp, rightTemp);
	edgeDetector.ProcessFrames(leftTemp, rightTemp, leftTemp, rightTemp);
	faceDetector.ProcessFrames(leftTemp, rightTemp, leftTemp, rightTemp);
	classifierLens.ProcessFrames(leftTemp, rightTemp, leftTemp, rightTemp);
	deepdreamLens.ProcessFrames(leftTemp, rightTemp, leftTemp, rightTemp);
	depthModule.ProcessFrames(leftTemp, rightTemp, leftTemp, rightTemp);
	TS_STOP_NIF("Frame Process");

	// Copy the final results to the mats the compositor will read from
	TS_START_NIF("Frame Out");
	leftTemp.copyTo(leftMat);
	rightTemp.copyTo(rightMat);
	TS_STOP_NIF("Frame Out");

	// Mark the frames as consumed
	left->MarkUsed();
	right->MarkUsed();
}


/**
Prints OpenCV debug information.
*/
void KinoCore::PrintCVDebugInfo() {
	Kino::app_log.AddLog("\n");
	Kino::app_log.AddLog("--- INFO ---\n");

	std::cout << "Using OpenCV " << CV_VERSION << std::endl;

	//std::cout << getBuildInformation();

#ifdef _DEBUG
	Kino::app_log.AddLog("Debug build\n");
	//std::cout << cv::getBuildInformation();
#else
	Kino::app_log.AddLog("Release build\n");
#endif

	// Get number of system cameras attached
	/*
	cv::VideoCapture temp_camera;
	int numSystemCameras = 0;
	for (int i = 0; i < 10; i++) {
		cv::VideoCapture temp_camera(i);
		numSystemCameras += temp_camera.isOpened() ? 1 : 0;
		temp_camera.release();
	}
	Kino::app_log.AddLog("System cameras attached: %i\n", numSystemCameras);
	*/

	bool openCLSupport = ocl::useOpenCL();
	Kino::app_log.AddLog("OpenCL acceleration is %s\n", cv::ocl::haveOpenCL() ? "available" : "not available");
	Kino::app_log.AddLog("OpenCL used: %s\n", openCLSupport ? "true" : "false");
#ifdef _DEBUG
	if (openCLSupport) {
		std::vector<cv::ocl::PlatformInfo> platforms;
		cv::ocl::getPlatfomsInfo(platforms);
		for (int i = 0; i < platforms.size(); i++) {
			Kino::app_log.AddLog("  Platform %i of %lu\n", i + 1, platforms.size());
			Kino::app_log.AddLog("    Name:     %s\n", platforms[i].name().c_str());
			Kino::app_log.AddLog("    Vendor:   %s\n", platforms[i].vendor().c_str());
			Kino::app_log.AddLog("    Device:   %i\n", platforms[i].deviceNumber());
			Kino::app_log.AddLog("    Version:  %s\n", platforms[i].version().c_str());
		}
	}
#endif
	Kino::app_log.AddLog("Optimized code support: %s\n", useOptimized() ? "true" : "false");
	Kino::app_log.AddLog("IPP support: %s\n", cv::ipp::useIPP() ? "true" : "false");
	Kino::app_log.AddLog("Threads used by OpenCV: %i\n", getNumThreads());
	Kino::app_log.AddLog("CPUs available: %i\n", cv::getNumberOfCPUs());

	Kino::app_log.AddLog("--- INFO ---\n\n");

}

/**
Draw the imgui windows and controls for this class and all the modules it manages.
*/
void KinoCore::DrawAllGUIs() {
	DrawGUI();

	cameraCalibrator.DrawGUI();
	faceDetector.DrawGUI();
	edgeDetector.DrawGUI();
	classifierLens.DrawGUI();
	deepdreamLens.DrawGUI();
	depthModule.DrawGUI();
}

void KinoCore::DrawGUI() {

}
