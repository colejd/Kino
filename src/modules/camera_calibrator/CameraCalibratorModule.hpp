#pragma once

#include "ofMain.h"

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"

#include "modules/ModuleCommon.hpp"
#include "gui/UsesGUI.hpp"
#include "KinoGlobals.hpp"

#include "camera/CameraCapture.hpp"

#include <string>

#include "CalibrationState.hpp"

using namespace std;

/**

Keep in mind the aspect ratio. It's 1 in the tutorial but I don't think that's correct.

Checkerboard squares are 26x26 mm.

*/

class CameraCalibratorModule : public ModuleCommon, public UsesGUI {
public:
	CameraCalibratorModule();
	~CameraCalibratorModule();

	void RegisterCameras(std::unique_ptr<CameraCapture> const & leftCapture, std::unique_ptr<CameraCapture> const & rightCapture);
	CalibrationState InitCalibrationState(std::unique_ptr<CameraCapture> const & cap, std::string id);

	void ProcessFrame(cv::InputArray in, cv::OutputArray out, string id);

	void DrawGUI() override;

private:
	// The ID of the currently calibrating camera. Set to empty string if no camera is calibrating
	string currentCalibrationID = "";

	CalibrationState leftCalibration;
	CalibrationState rightCalibration;

};