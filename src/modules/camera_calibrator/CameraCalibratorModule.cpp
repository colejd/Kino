#include "CameraCalibratorModule.hpp"

using namespace cv;

CameraCalibratorModule::CameraCalibratorModule() {

}

CameraCalibratorModule::~CameraCalibratorModule() {

}

/**
Registers camera captures for calibration and distortion. Builds the map.
*/
void CameraCalibratorModule::RegisterCameras(std::unique_ptr<CameraCapture> const& leftCapture, std::unique_ptr<CameraCapture> const& rightCapture) {
	leftCalibration = InitCalibrationState(leftCapture, "LEFT");
	rightCalibration = InitCalibrationState(rightCapture, "RIGHT");
}

CalibrationState CameraCalibratorModule::InitCalibrationState(std::unique_ptr<CameraCapture> const & cap, std::string id) {
	CalibrationState calibration = CalibrationState();

	if (cap->IsInitialized()) {
		// Load the calibration from the disk if it exists.
		string calibrationBaseFilename = cap->DeviceName() + "_" + id;
		string calibrationFilePath = ofToDataPath("calibration/" + calibrationBaseFilename + ".calibration");
		calibration.LoadFromFile(calibrationFilePath);
		if (!calibration.complete) {
			Kino::app_log.AddLog("No calibration for %s exists.\n", calibrationBaseFilename.c_str());
		}
	}

	return calibration;
}

void CameraCalibratorModule::ProcessFrame(cv::InputArray in, cv::OutputArray out, string id) {
	if (IsEnabled()) {

		CalibrationState* calibration;
		if (id == "LEFT") calibration = &leftCalibration;
		else if(id == "RIGHT") calibration = &rightCalibration;
		else {
			Kino::app_log.AddLog("Calibration ID must be LEFT or RIGHT");
			return;
		}

		// If the model is fully formed, do the distortion.
		if (calibration->complete) {
			//cv::Mat temp = latestStep.clone();
			//undistort(temp, latestStep, model.intrinsic, model.distCoeffs);

			// This might cause some problems
			cv::Mat temp;
			in.copyTo(temp);
			undistort(temp, out, calibration->K, calibration->D);
		}

		// Otherwise, run calibration.
		else {

			cv::Mat latestStep;
			in.copyTo(latestStep);

			// Only run calibration if the ID given matches the desired ID
			if (id == currentCalibrationID) {
				calibration->ProcessImage(in, latestStep);
				if (calibration->complete) id = ""; // Quit the calibration
			}

			latestStep.copyTo(out);
		}

	}
}

void CameraCalibratorModule::DrawGUI() {
	if (showGUI) {
		ImGui::Begin("Camera Calibrator", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{

			if (ImGui::Button("Calibrate Left")) {
				currentCalibrationID = "LEFT";
				// TODO: Invalidate the calibration
			}

			if (ImGui::Button("Calibrate Right")) {
				//currentCalibrationID = "Right";
				// TODO: Invalidate the calibration
			}

			// Draw info about left calibration
			CalibrationState leftState = leftCalibration;
			ImGui::BulletText("Left");
			ImGui::Text("%-15s (%i, %i)","Board Size:", leftState.board_size.width, leftState.board_size.height);
			ImGui::Text("%-15s %.2fmm", "Square Size:", leftState.square_size);
			ImGui::Text("%-15s %d / %d", "Captures:", leftState.numCaptures, leftState.num_imgs);

			

		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}