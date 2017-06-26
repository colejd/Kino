#include "CameraCalibratorModule.hpp"

CameraCalibratorModule::CameraCalibratorModule() {

}

CameraCalibratorModule::~CameraCalibratorModule() {

}

/**
Registers camera captures for calibration and distortion. Builds the map.
*/
void CameraCalibratorModule::RegisterCameras(unique_ptr<CameraCapture> const& leftCapture, unique_ptr<CameraCapture> const& rightCapture) {
	InitCalibrationState(leftCapture, "LEFT");
	InitCalibrationState(rightCapture, "RIGHT");
}

void CameraCalibratorModule::InitCalibrationState(unique_ptr<CameraCapture> const& cap, string id) {
	calibrations[id] = CalibrationState();

	if (cap->IsInitialized()) {
		calibrations[id].hasCapture = true;
		// Load the calibration from the disk if it exists.
		calibrations[id].unique_id = cap->DeviceName() + "_" + id;
		calibrations[id].LoadFromFile();
		if (!calibrations[id].complete) {
			Kino::app_log.AddLog("No calibration for %s exists.\n", calibrations[id].unique_id.c_str());
		}
	}
}

void CameraCalibratorModule::ProcessFrame(cv::InputArray in, cv::InputOutputArray out, string id) {
	if (IsEnabled()) {
		TS_START_NIF("Camera Calibrator");

		CalibrationState* calibration = &(calibrations[id]);

		// If the model is fully formed, do the distortion.
		if (calibration->complete) {
			TS_SCOPE("Undistort");
			calibration->UndistortImage(in, out);
		}

		// Otherwise, run calibration.
		else {
			// Only run calibration if the ID given matches the desired ID
			if (id == currentCalibrationID) {
				TS_SCOPE("Process Calibration Image");
				calibration->ProcessImage(in, out);
				if (calibration->complete) {
					StopCalibrating(id); // Quit the calibration
				}
			}
		}

		in.copyTo(id == "LEFT" ? lastLeftMat : lastRightMat);

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
			// Draw info about left calibration
			bool leftHasCapture = calibrations["LEFT"].hasCapture;
			if (leftHasCapture) {
				ImGui::Spacing();
				DrawCalibrationStatePanel("LEFT");
			}

			// Draw info about right calibration
			bool rightHasCapture = calibrations["RIGHT"].hasCapture;
			if (rightHasCapture) {
				ImGui::Spacing();
				DrawCalibrationStatePanel("RIGHT");
			}

			if (leftHasCapture && rightHasCapture) {
				if (ImGui::Button("Save Capture Pairs")) {
					string basePath = ofToDataPath("calibration/stereo_images/");
					imwrite(basePath + "Capture" + std::to_string(capCount) + "_LEFT.png", lastLeftMat);
					imwrite(basePath + "Capture" + std::to_string(capCount) + "_RIGHT.png", lastRightMat);
					capCount += 1;
					Kino::app_log.AddLog("Stereo image pair written to data/calibration/stereo_images/");
				}
			}
			

			

		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}

void CameraCalibratorModule::DrawCalibrationStatePanel(string id) {

	CalibrationState& calibration = calibrations[id];

	bool calibratingOther = (currentCalibrationID != id && currentCalibrationID != "");
	if (calibratingOther) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style

	ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
	ImGui::BeginChild((id + "Sub").c_str(), ImVec2(250, 150), true);
	{
		// Header
		ImGui::Text(id.c_str());

		ImVec4 calibratingColor;
		string calibratingString;
		if (calibration.complete) {
			calibratingColor = ImVec4(0.0, 1.0, 0.0, 1.0);
			calibratingString = "Calibrated";
		}
		else if (currentCalibrationID == id) {
			calibratingColor = ImVec4(1.0, 1.0, 0.0, 1.0);
			calibratingString = "Calibrating...";
		}
		else {
			calibratingColor = ImVec4(1.0, 0.0, 0.0, 1.0);
			calibratingString = "Uncalibrated";
		}
		ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(calibratingString.c_str()).x - 10);
		ImGui::TextColored(calibratingColor, calibratingString.c_str());

		ImGui::Separator();
		ImGui::Spacing();

		// Body

		if (currentCalibrationID == "") {
			if (ImGui::Button("Calibrate")) {
				StartCalibrating(id);
			}
		}
		else if (currentCalibrationID == id){
			if (ImGui::Button("Stop Calibrating")) {
				StopCalibrating(id);
			}
		}

		ImGui::Text("%-20s (%i x %i)", "Board Size:", calibration.boardSize.width, calibration.boardSize.height);
		ImGui::Text("%-20s %.2fmm", "Square Size:", calibration.squareSize);
		ImGui::Text("%-20s %.3f", "Reprojection Error:", calibration.reprojectionError);
		ShowHelpMarker("Should be as close to 0 as possible.");

		if (currentCalibrationID == id) {
			ImGui::Text("%-20s %d / %d", "Captures:", calibration.numCaptures, calibration.capturesRequired);
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

	if (calibratingOther) ImGui::PopStyleVar(); //Pop disabled style

}

void CameraCalibratorModule::StartCalibrating(string id) {
	calibrations[id].Reset();
	currentCalibrationID = id;
}


void CameraCalibratorModule::StopCalibrating(string id) {
	currentCalibrationID = "";
}


