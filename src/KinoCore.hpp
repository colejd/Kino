#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/ocl.hpp>

#include "KinoGlobals.hpp"

#include "gui/UsesGUI.hpp"

#include <modules/ModulePipeline.hpp>
#include <modules/edge_detector/EdgeDetectorModule.hpp>
#include <modules/face_detector/FaceDetectorModule.hpp>
#include <modules/image_classifier/ClassifierLens.hpp>
#include <modules/deepdream/DeepDreamLens.hpp>
#include <modules/camera_calibrator/CameraCalibratorModule.hpp>
#include <modules/depth_mapper/StereoDepthModule.hpp>

#include "camera/CameraCapture.hpp"

#include "ofxTimeMeasurements.h"

class Frame;

/*
Holds the code for grabbing images from a camera and performing
OpenCV operations on them. Should not have any engine-specific code
(i.e. OpenFrameworks or Cinder code).
*/
class KinoCore : public UsesGUI {
public:

	KinoCore();
	~KinoCore();

	void Setup();
	void Update();
	void ConsumeFrames();

	void PrintCVDebugInfo();

	void DrawGUI();
	void DrawAllGUIs();

	ModulePipeline modulePipeline;
	CameraCalibratorModule cameraCalibrator;
	EdgeDetectorModule edgeDetector;
	FaceDetectorModule faceDetector;
	ClassifierLens classifierLens;
	DeepDreamLens deepdreamLens;
	StereoDepthModule depthModule;

	cv::UMat leftMat = cv::UMat();
	cv::UMat rightMat = cv::UMat();

	std::unique_ptr<CameraCapture> capture1;
	std::unique_ptr<CameraCapture> capture2;

	void ProcessCapture(std::unique_ptr<CameraCapture> const& cap, string id);

	bool pauseCaptureUpdates = false;
	bool swapSides = false;

	std::map<string, Frame> framebuffer;

	bool demoMode;
private:
	
};

class Frame {
public:
	cv::UMat data;

	// Timing info
	float fps = 0;
	float avgFPS = 0;
	int numFrames = 0;

	Frame() {};
	~Frame() {};

	void MarkReady() {
		ready = true;

		numFrames += 1;

		float time = ofGetElapsedTimef();
		float delta = time - lastReadyTime;
		fps = 1.0 / delta;

		avgFPS += (fps - avgFPS) / numFrames;

		lastReadyTime = time;
	};
	void MarkUsed() { ready = false; };
	bool IsReady() { return ready; };

private:
	bool ready = false;

	float lastReadyTime;
};