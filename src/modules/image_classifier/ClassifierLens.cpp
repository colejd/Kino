#include "ClassifierLens.hpp"

using namespace std;
using namespace cv;

//#include <Windows.h>

ClassifierLens::ClassifierLens() {

	//darknet.init("data/cfg/vgg-conv.cfg", "data/vgg-conv.weights");

	if (ConfigHandler::GetValue("YOLO.PRELOAD", false).asBool()) {
		InitFromConfig();
		initialized = true;
	}

	trackerLeft = TrackerKCF::create();
	trackerRight = TrackerKCF::create();

}

ClassifierLens::~ClassifierLens() {
}

/**
Loads ofxDarknet from the paths specified by YOLO.USED_CONFIG in the config file.
*/
void ClassifierLens::InitFromConfig() {
	// Construct the key to the desired config from YOLO.USED_CONFIG
	string config_key = "YOLO.CONFIGS." + ConfigHandler::GetValue("YOLO.USED_CONFIG", "").asString();
	Kino::app_log.AddLog("Loading YOLO config: %s\n", config_key.c_str());
	// Find paths specified in the config given by the key
	cfg_file = ConfigHandler::GetValue(config_key + ".CFG_FILE", "").asString();
	weights_file = ConfigHandler::GetValue(config_key + ".WEIGHTS_FILE", "").asString();
	names_list = ConfigHandler::GetValue(config_key + ".NAMES_LIST", "").asString();

	string basePath = ConfigHandler::GetValue("YOLO.BASE_PATH", "").asString();
	darknet.init(ofToDataPath(basePath + cfg_file),
		ofToDataPath(basePath + weights_file),
		names_list == "" ? "" : ofToDataPath(basePath + names_list));
}

void ClassifierLens::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	if (IsEnabled() && !inLeft.empty() && !inRight.empty()) {
		TS_SCOPE("Classifier Lens");

		// Init Darknet on demand if not already loaded
		if (!initialized) {
			InitFromConfig();
			initialized = true;
		}

		Mat drawMatLeft, drawMatRight;
		inLeft.copyTo(drawMatLeft);
		inRight.copyTo(drawMatRight);

		Mat analysisMatLeft, analysisMatRight;
		// Downsample analysisMat if requested
		if (doDownsampling) {
			cv::resize(inLeft, analysisMatLeft, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
			cv::resize(inRight, analysisMatRight, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
		}
		else {
			inLeft.copyTo(analysisMatLeft);
			inRight.copyTo(analysisMatRight);
		}

		lastSizeLeft = analysisMatLeft.size();
		lastSizeRight = analysisMatRight.size();

		// Step 1: Acquire targets

		// TODO: Not sure these two steps are needed since customizing ofxDarknet to accept mats natively
		cv::cvtColor(analysisMatLeft, analysisMatLeft, CV_BGR2RGB);
		cv::cvtColor(analysisMatRight, analysisMatRight, CV_BGR2RGB);

		TS_START_NIF("YOLO");
		detectionsLeft = darknet.yolo(analysisMatLeft, threshold);
		detectionsRight = darknet.yolo(analysisMatRight, threshold);
		TS_STOP_NIF("YOLO");

		TS_START_NIF("Draw Detections");
		DrawDetections(drawMatLeft, detectionsLeft);
		DrawDetections(drawMatRight, detectionsRight);
		TS_STOP_NIF("Draw Detections");


		// Step 2: Track targets
		// Need some kind of tracker dispatcher that creates trackers for each detected object,
		// but doesn't create new ones for objects that are already tracked (similar ROIs)



		drawMatLeft.copyTo(outLeft);
		drawMatRight.copyTo(outRight);

	}
	else {
		detectionsLeft.clear();
		detectionsRight.clear();
	}
}

void ClassifierLens::DrawDetections(InputOutputArray mat, std::vector< detected_object > detections) {
	for (detected_object d : detections)
	{

		int lineThickness = 4;

		Scalar color = CV_RGB(0, 255, 0);

		cv::Rect rect = cv::Rect(d.rect.x, d.rect.y, d.rect.width, d.rect.height);
		if (doDownsampling) { // Rescale the rect to match original size if downsampling was performed
			rect = cv::Rect(d.rect.x / downSampleRatio, d.rect.y / downSampleRatio, d.rect.width / downSampleRatio, d.rect.height / downSampleRatio);
		}

		cv::Point origin(rect.x, rect.y);
		cv::Point opposite(rect.x + rect.width, rect.y + rect.height);
		// Draw the bounding box
		// Random color: Scalar(rand() & 255, rand() & 255, rand() & 255)
		cv::rectangle(mat, origin, opposite, color, lineThickness, LINE_4);

		// Draw the label in the bounding box
		int fontface = cv::FONT_HERSHEY_DUPLEX;
		double scale = 0.8;
		int textThickness = 1;
		int baseline = 0;
		string label = d.label;

		cv::Size text = cv::getTextSize(label, fontface, scale, textThickness, &baseline);

		// Interior label
		cv::Point rectOrigin = cv::Point(rect.x + (lineThickness / 2.0), rect.y + text.height + (lineThickness / 2.0));
		cv::rectangle(mat, rectOrigin + cv::Point(0, baseline), rectOrigin + cv::Point(text.width, -text.height), color, CV_FILLED);

		// Exterior label
		//cv::Point origin = cv::Point(rect.x - (lineThickness / 2.0), rect.y - text.height - (lineThickness / 2.0) + baseline); // Exterior label
		//cv::rectangle(drawingMat, origin + cv::Point(0, baseline), origin + cv::Point(text.width, -text.height), color, CV_FILLED);

		cv::putText(mat, label, rectOrigin, fontface, scale, CV_RGB(0, 0, 0), textThickness, LINE_AA);

	}
}

void ClassifierLens::DrawGUI() {
	if (showGUI) {
		ImGui::Begin("Classifier", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style

		// GUI stuff goes here
		{
			// Controls

			ImGui::SliderFloat("Threshold", &threshold, 0.01, 1.0, "%.2f%");
			//ShowHelpMarker("Test help");

			ImGui::Checkbox("Downsample", &doDownsampling);
			if (!doDownsampling) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
			ImGui::SliderFloat("Downsample Ratio", &downSampleRatio, 0.01f, 1.0f, "%.2f");
			ShowHelpMarker("Multiplier for decreasing the resolution of the processed image.");
			if (!doDownsampling) ImGui::PopStyleVar(); //Pop disabled style
			ImGui::Text("Dimensions - Left: (%i, %i), Right: (%i, %i)", lastSizeLeft.width, lastSizeLeft.height, lastSizeRight.width, lastSizeRight.height);

			// Info

			if (ImGui::TreeNode("Darknet Info")) {
				if (initialized) {
					ImGui::Text("%-10s %s", "Preset: ", ConfigHandler::GetValue("YOLO.USED_CONFIG", "").asString().c_str());
					ImGui::Text("%-10s %s", "Config:", Paths::GetFileNameFromPath(cfg_file).c_str());
					ImGui::Text("%-10s %s", "Weights:", Paths::GetFileNameFromPath(weights_file).c_str());

					string namesList = Paths::GetFileNameFromPath(names_list);
					ImGui::Text("%-10s %s", "Names:", namesList == "" ? "None" : namesList.c_str());
				}
				else {
					ImGui::TextColored(ImColor(255, 255, 0), "No configuration loaded.");
				}

				ImGui::TreePop();
			}

			ImGui::Spacing();
			ImGui::Spacing();

			// Output Box

			int maxLines = 10; // Number of lines in the scroll box
			int maxCharsInLabel = 15; // Number of characters in the label

			float glyph_width = ImGui::CalcTextSize("F").x;

			// Draw header
			ImGui::TextColored(ImColor(255, 255, 0), " %*s ", -maxCharsInLabel, "Label"); // spacing is deliberate
			ImGui::SameLine();
			ImGui::Text(" %s", "Confidence");

			// Draw scroll box
			ImGui::BeginChild("##scrolling", ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * maxLines), true, 0);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 5));

			bool drawLine = true;
			// Fill scroll box with items if enabled
			if (enabled) {

				ImGuiListClipper clipper(detectionsLeft.size(), ImGui::GetTextLineHeightWithSpacing());
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {

					detected_object d = detectionsLeft[i];
					ImGui::Text("%*s ", -maxCharsInLabel, d.label.c_str()); // Left pad the line with maxCharsInLine length

					if (drawLine) { // Draw the line only once
						ImGui::SameLine();
						ImVec2 screen_pos = ImGui::GetCursorScreenPos();
						ImGui::GetWindowDrawList()->AddLine(ImVec2(screen_pos.x - glyph_width, screen_pos.y - 9999), ImVec2(screen_pos.x - glyph_width, screen_pos.y + 9999), ImColor(ImGui::GetStyle().Colors[ImGuiCol_Border]));
						drawLine = false;
					}

					ImGui::SameLine();
					ImGui::Text(" %.2f%", d.probability * 100);

				}
				clipper.End();
			}

			ImGui::PopStyleVar(2);
			ImGui::EndChild();

		}

		// End GUI stuff
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}

}