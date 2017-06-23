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
		string calibrationBaseFilename = cap->DeviceName() + "_" + id;
		string calibrationFilePath = ofToDataPath("calibration/" + calibrationBaseFilename + ".calibration");
		calibrations[id].LoadFromFile(calibrationFilePath);
		if (!calibrations[id].complete) {
			Kino::app_log.AddLog("No calibration for %s exists.\n", calibrationBaseFilename.c_str());
		}
	}
}

void CameraCalibratorModule::ProcessFrame(cv::InputArray in, cv::OutputArray out, string id) {
	if (IsEnabled()) {

		CalibrationState* calibration = &(calibrations[id]);

		// If the model is fully formed, do the distortion.
		if (calibration->complete) {
			calibration->UndistortImage(in, out);
		}

		// Otherwise, run calibration.
		else {

			cv::Mat latestStep;
			in.copyTo(latestStep);

			// Only run calibration if the ID given matches the desired ID
			if (id == currentCalibrationID) {
				calibration->ProcessImage(in, latestStep);
				if (calibration->complete) StopCalibrating(id); // Quit the calibration
			}

			latestStep.copyTo(out);
		}

	}
}

void CameraCalibratorModule::DrawGUI() {
	if (showGUI) {

		/*ImGui::BeginMainMenuBar();
		float menuBarHeight = ImGui::GetWindowSize().y;
		ImGui::EndMainMenuBar();

		ImVec2 size = ImVec2(300, ImGui::GetIO().DisplaySize.y);
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, menuBarHeight));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::Begin("Camera Calibrator", &showGUI, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings);*/

		ImGui::Begin("Camera Calibrator", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{
			// Draw info about left calibration
			if (calibrations["LEFT"].hasCapture) {
				ImGui::Spacing();
				DrawCalibrationStatePanel("LEFT", calibrations["LEFT"]);
			}

			// Draw info about right calibration
			if (calibrations["RIGHT"].hasCapture) {
				ImGui::Spacing();
				DrawCalibrationStatePanel("RIGHT", calibrations["RIGHT"]);
			}
			

			

		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
		//ImGui::PopStyleVar();
	}
}

void CameraCalibratorModule::DrawCalibrationStatePanel(string id, CalibrationState& calibration) {
	
	ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
	ImGui::BeginChild((id + "Sub").c_str(), ImVec2(200, 150), true);
	{
		ImGui::Text(id.c_str());
		ImGui::Separator();
		ImGui::Spacing();

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

		if (calibration.complete) ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), "Calibrated");
		else if (currentCalibrationID == id) ImGui::TextColored(ImVec4(1.0, 1.0, 0.0, 1.0), "Calibrating...");
		else ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), "Uncalibrated");
		ImGui::Text("%-15s (%i, %i)", "Board Size:", calibration.board_size.width, calibration.board_size.height);
		ImGui::Text("%-15s %.2fmm", "Square Size:", calibration.square_size);

		if (currentCalibrationID == id) {
			ImGui::Text("%-15s %d / %d", "Captures:", calibration.numCaptures, calibration.capturesRequired);
		}
	}
	ImGui::EndChild();
	ImGui::PopStyleVar();

}

void CameraCalibratorModule::StartCalibrating(string id) {
	calibrations[id].Reset();
	currentCalibrationID = id;
}


void CameraCalibratorModule::StopCalibrating(string id) {
	currentCalibrationID = "";
}


/**
ImVec2 size = ImVec2(300, ImGui::GetIO().DisplaySize.y);
ImGui::SetNextWindowSize(size);
const ImGuiWindowFlags flags = (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar);
const float oldWindowRounding = ImGui::GetStyle().WindowRounding; ImGui::GetStyle().WindowRounding = 0;
const bool visible = ImGui::Begin("imguidock window (= lumix engine's dock system)", NULL, ImVec2(0, 0), 1.0f, flags);
ImGui::GetStyle().WindowRounding = oldWindowRounding;
if (visible) {
ImGui::BeginDockspace();

ImGui::SetNextDock(ImGuiDockSlot_None);// optional
ImGui::BeginDock("Test dock 1");
ImGui::EndDock();

ImGui::EndDockspace();
}
ImGui::End();
*/