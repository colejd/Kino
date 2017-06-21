#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"

#include "modules/ModuleCommon.hpp"
#include "gui/UsesGUI.hpp"

using namespace std;

class CameraCalibratorModule : public ModuleCommon, public UsesGUI {
public:
	CameraCalibratorModule();
	~CameraCalibratorModule();

	void ProcessFrame(cv::InputArray in, cv::OutputArray out);

	void DrawGUI() override;

private:

};
