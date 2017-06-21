#include "KinoCore.hpp"

using namespace cv;

KinoCore::KinoCore()
{
	Setup();
}

KinoCore::~KinoCore()
{
}

void KinoCore::Setup()
{
	cv::ocl::setUseOpenCL(ConfigHandler::GetValue("OPENCV.USE_OPENCL", false).asBool());
	bool demoMode = ConfigHandler::GetValue("DEMO_SETTINGS.ACTIVE", false).asBool();

	PrintCVDebugInfo();

	// Initialize modules
	edgeDetector = EdgeDetectorModule();
	faceDetector = FaceDetectorModule();

	// Initialize captures
	capture1 = make_unique<CameraCapture>();
	capture2 = make_unique<CameraCapture>();

	if (demoMode) {
		// Demo mode: show only input from one capture source in fullscreen
		string cameraType = ConfigHandler::GetValue("DEMO_SETTINGS.CAMERA_MODE", "").asString();
		int cameraIndex = ConfigHandler::GetValue("DEMO_SETTINGS.CAMERA_INDEX", 0).asInt();

		Kino::app_log.AddLog("Demo Mode active (mode: %s)\n", cameraType);

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

}

void KinoCore::Update()
{
	ProcessCapture(capture1, leftMat, "Left");
	ProcessCapture(capture2, rightMat, "Right");
}

/**
If the capture has a new frame ready, run the frame through each module. The
results are written to `output`.
*/
void KinoCore::ProcessCapture(std::unique_ptr<CameraCapture> const& cap, cv::OutputArray output, string id)
{
	if (!cap->IsInitialized()) {
		return;
	}

	string capTimerID = "Capture " + id + " Update";
	TS_START_NIF(capTimerID);

	if (cap->IsThreaded() == false) {
		//If the capture isn't threaded, we need to manually update it before querying it here.
		TS_START_NIF("Update");
		cap->UpdateCapture();
		TS_STOP_NIF("Update");
	}
	if (cap->FrameIsReady()) {
		//cv::Mat rawFrame(cap->GetWidth(), cap->GetHeight(), CV_8UC3, cap->GetLatestFrame().data);

		if (!pauseCaptureUpdates) {
			TS_START_NIF("Frame Grab");
			rawFrame = cap->RetrieveCapture().clone();
			TS_STOP_NIF("Frame Grab");
		}

		cv::Mat intermediate;
		rawFrame.copyTo(intermediate);

		//cv::UMat rawFrameGPU;
		//rawFrame.copyTo(rawFrameGPU);

		// Process through each lens
		edgeDetector.ProcessFrame(intermediate, intermediate);
		faceDetector.ProcessFrame(intermediate, intermediate);
		classifierLens.ProcessFrame(intermediate, intermediate);
		deepdreamLens.ProcessFrame(intermediate, intermediate);

		TS_START_NIF("Frame Copy");
		//output = rawFrame.clone();
		intermediate.copyTo(output);
		TS_STOP_NIF("Frame Copy");

		//leftMat = rawFrameGPU.getMat(0).clone();

		//More or less a mutex unlock for capture->GetLatestFrame()
		//capture1->MarkFrameUsed();
		//rawFrame.release();
		//rawFrameGPU.release();
	}
	TS_STOP_NIF(capTimerID);
}

/**
Prints OpenCV debug information.
*/
void KinoCore::PrintCVDebugInfo()
{
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

	faceDetector.DrawGUI();
	edgeDetector.DrawGUI();
	classifierLens.DrawGUI();
	deepdreamLens.DrawGUI();
}

void KinoCore::DrawGUI() {

}
