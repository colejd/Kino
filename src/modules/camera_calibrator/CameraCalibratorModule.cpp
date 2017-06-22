#include "CameraCalibratorModule.hpp"

using namespace cv;

CameraCalibratorModule::CameraCalibratorModule() {

}

CameraCalibratorModule::~CameraCalibratorModule() {

}

void CameraCalibratorModule::ProcessFrame(cv::InputArray in, cv::OutputArray out, CameraCapture::CAPTURE_TYPE captureType, string id) {
	if (IsEnabled()) {
		cv::Mat latestStep;
		in.copyTo(latestStep);

		// If calibrating, run calibration per-frame routine


		// Else if distorting, run distortion per-frame routine
		

		latestStep.copyTo(out);
	}
	else {
		in.copyTo(out);
	}
}

void CameraCalibratorModule::DrawGUI() {
	if (showGUI) {
		ImGui::Begin("Camera Calibrator", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		//if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{

			

		}
		//End main content
		//if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}

void CameraCalibratorModule::SetMode(Mode mode) {
	currentMode = mode;

	if (mode == Mode::Calibrate) {
		// Load everything the calibration needs to run
	}
	else if (mode == Mode::Distort) {
		// Load the distortion file
	}

}
