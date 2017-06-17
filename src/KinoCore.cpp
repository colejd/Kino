#include "KinoCore.hpp"

using namespace cv;

KinoCore::KinoCore()
{
	Setup();
}

KinoCore::~KinoCore()
{
	delete capture1;
	delete capture2;
} 

void KinoCore::Setup()
{
	cv::ocl::setUseOpenCL(ConfigHandler::GetValue("USE_OPENCL", false).asBool());

	PrintCVDebugInfo();

	edgeDetector = EdgeDetectorModule();
	faceDetector = FaceDetectorModule();

	capture1 = new CameraCapture();
	capture2 = new CameraCapture();

	bool demoMode = ConfigHandler::GetValue("WEBCAM_DEMO_MODE", false).asBool();
	if (demoMode) {
		// Demo mode: show only input from the webcam and make full screen
		//capture1->StartCapturing(0, CameraCapture::DEVICE_TYPE::GENERIC, true);
		capture1->StartFakeCapture(ofToDataPath("video/kitty.MOV"), true);
		//capture1->StartCapturing(0, CameraCapture::CAPTURE_TYPE::PS3EYE, true);

	}
	else {
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
Runs the latest frame from the capture through each module when there is
a new frame and returns the result as a cv::Mat.
*/
void KinoCore::ProcessCapture(CameraCapture *cap, cv::OutputArray output, string id)
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

		TS_START_NIF("Frame Grab");
		cv::Mat rawFrame = cap->RetrieveCapture().clone();
		TS_STOP_NIF("Frame Grab");

		//cv::UMat rawFrameGPU;
		//rawFrame.copyTo(rawFrameGPU);

		// Process through each lens
		if (edgeDetector.enabled) {
			TS_START_NIF("Edge Detector");
			edgeDetector.ProcessFrame(rawFrame, rawFrame);
			TS_STOP_NIF("Edge Detector");
		}
		if (faceDetector.enabled) {
			TS_START_NIF("Face Detector");
			faceDetector.ProcessFrame(rawFrame, rawFrame);
			TS_STOP_NIF("Face Detector");
		}
		if (classifierLens.enabled) {
			TS_START_NIF("Classifier Lens");
			classifierLens.ProcessFrame(rawFrame, rawFrame);
			TS_STOP_NIF("Classifier Lens");
		}

		TS_START_NIF("Frame Copy");
		//output = rawFrame.clone();
		rawFrame.copyTo(output);
		TS_STOP_NIF("Frame Copy");

		//leftMat = rawFrameGPU.getMat(0).clone();

		//More or less a mutex unlock for capture->GetLatestFrame()
		//capture1->MarkFrameUsed();
		rawFrame.release();
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
			for (int i = 0; i<platforms.size(); i++) {
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
	Kino::app_log.AddLog("CPUs available: %2.i\n", cv::getNumberOfCPUs());

	Kino::app_log.AddLog("--- INFO ---\n\n");

}

/**
Draw the imgui windows and controls for this class and all the modules it manages.
*/
void KinoCore::DrawAllGUIs() {
	DrawGUI();

	// TODO: Store each module is a vector and call DrawGUI on all of them.
	faceDetector.DrawGUI();
	edgeDetector.DrawGUI();
	classifierLens.DrawGUI();
}

void KinoCore::DrawGUI() {

}
