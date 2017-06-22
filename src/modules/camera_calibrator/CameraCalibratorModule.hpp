#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"

#include "modules/ModuleCommon.hpp"
#include "gui/UsesGUI.hpp"

#include "camera/CameraCapture.hpp"

using namespace std;

/**
Two modes:

Calibrate:
Performs calibration on the selected camera and stores the result as a file

Distort:
Reads the calibration file and distorts the image.

*/

class CameraCalibratorModule : public ModuleCommon, public UsesGUI {
public:
	CameraCalibratorModule();
	~CameraCalibratorModule();

	void ProcessFrame(cv::InputArray in, cv::OutputArray out, CameraCapture::CAPTURE_TYPE captureType, string id);

	void DrawGUI() override;

	enum class Mode {
		Calibrate,
		Distort
	};

	void SetMode(Mode mode);

private:
	Mode currentMode;
};
