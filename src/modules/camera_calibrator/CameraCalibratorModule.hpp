#pragma once


#include <string>

#include "ofMain.h"

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>

#include <modules/ModuleCommon.hpp>
#include <gui/UsesGUI.hpp>
#include <KinoGlobals.hpp>
#include <ofxTimeMeasurements.h>

#include <camera/CameraCapture.hpp>
#include "CalibrationState.hpp"

using namespace std;
using namespace cv;

/**
Responsible for ingesting images from the cameras, determining their distortion parameters,
and then undistorting the images.

Where `id` is used, either "LEFT" or "RIGHT" is expected.
*/
class CameraCalibratorModule : public ModuleCommon, public UsesGUI {
public:
	CameraCalibratorModule();
	~CameraCalibratorModule();

	// Constructs the mapping of cameras to `CalibrationState` objects. Call this before doing anything else with this class!
	void RegisterCameras(unique_ptr<CameraCapture> const& leftCapture, unique_ptr<CameraCapture> const & rightCapture);

	// Runs calibration or undistortion on `in` and writes the result to `out`. ID is expected to be "LEFT" or "RIGHT".
	void ProcessFrame(cv::InputArray in, cv::InputOutputArray out, string id);

	void StartCalibrating(string id);
	void StopCalibrating(string id);

	void DrawGUI() override;
	void DrawCalibrationStatePanel(string id);
private:
	// The ID of the currently calibrating camera. Set to empty string if no camera is calibrating.
	string currentCalibrationID = "";

	// Mapping of `CalibrationState` objects to string identifiers ("LEFT" or "RIGHT").
	std::map<string, CalibrationState> calibrations;

	// Adds a `CalibrationState` for the given capture to the `calibrations` map.
	void InitCalibrationState(unique_ptr<CameraCapture> const& cap, string id);

};