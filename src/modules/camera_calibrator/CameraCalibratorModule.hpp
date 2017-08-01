#pragma once


#include <string>

#include "ofMain.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <modules/ModuleCommon.hpp>
#include <KinoGlobals.hpp>
#include <ofxTimeMeasurements.h>
#include "config/ConfigHandler.hpp"

#include <camera/CameraCapture.hpp>
#include "CalibrationState.hpp"
#include "StereoCalibrationState.hpp"

using namespace std;
using namespace cv;

/**
Responsible for ingesting images from the cameras, determining their distortion parameters,
and then undistorting the images.

Where `id` is used, either "LEFT" or "RIGHT" is expected.
*/
class CameraCalibratorModule : public ModuleCommon {
public:
	CameraCalibratorModule();
	~CameraCalibratorModule();

	std::string GetName() override {
		return "Camera Calibrator";
	}

	// Constructs the mapping of cameras to `CalibrationState` objects. Call this before doing anything else with this class!
	void RegisterCameras(unique_ptr<CameraCapture> const& leftCapture, unique_ptr<CameraCapture> const & rightCapture);

	// Runs calibration or undistortion on `in` and writes the result to `out`. ID is expected to be "LEFT" or "RIGHT".
	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) override;

	void StartCalibrating(string id);
	void StopCalibrating(string id);

	void DrawGUI() override;
	void DrawCalibrationStatePanel(string id);
	void DrawStereoCalibrationPanel();

	enum class Mode {
		INDIVIDUAL,
		STEREO
	};
	Mode currentMode = Mode::INDIVIDUAL;

private:
	bool moduleCanRunStereo = false;

	// The ID of the currently calibrating camera. Set to empty string if no camera is calibrating.
	string currentlyCalibratingID = "";

	// Mapping of `CalibrationState` objects to string identifiers ("LEFT" or "RIGHT").
	std::map<string, CalibrationState> calibrations;
	StereoCalibrationState stereoCalibration;

	// Adds a `CalibrationState` for the given capture to the `calibrations` map.
	void InitCalibrationState(unique_ptr<CameraCapture> const& cap, string id);

	int capCount = 1;
	int leftCapCount = 1;
	int rightCapCount = 1;

	bool captureMode = false;

	// Cached mats saved to disk when requested by gui
	Mat lastLeftMat;
	Mat lastRightMat;

	void DrawCheckerboardPreview(InputArray in, OutputArray out);

	bool drawEpipolarLines = false;

	Ptr<FeatureDetector> detector = ORB::create();
	Ptr<DescriptorExtractor> extractor = ORB::create();
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce");

	void VisualizeEpipolarLines(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight);

};