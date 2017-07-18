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

	leftTracker = TrackedObjectManager();
	rightTracker = TrackedObjectManager();

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

		if (frameCounter == 0) {

			// Step 1: Acquire targets
			TS_START_NIF("YOLO");
			detectionsLeft = Classify(analysisMatLeft);
			detectionsRight = Classify(analysisMatRight);
			TS_STOP_NIF("YOLO");

			if (useTracking) {
				// Step 2: Track targets
				leftTracker.IngestDetections(detectionsLeft);
				rightTracker.IngestDetections(detectionsRight);
			}

		}
		if (useTracking) {
			// Use frameskipping only when trackers are used
			frameCounter += 1;
			if (frameCounter >= frameskip) frameCounter = 0;
		}

		if (useTracking) {

			leftTracker.UpdateTracking(analysisMatLeft);
			rightTracker.UpdateTracking(analysisMatRight);

			leftTracker.DrawBoundingBoxes(drawMatLeft);
			rightTracker.DrawBoundingBoxes(drawMatRight);

		}

		else {
			TS_START_NIF("Draw Detections");
			DrawDetections(drawMatLeft, detectionsLeft);
			DrawDetections(drawMatRight, detectionsRight);
			TS_STOP_NIF("Draw Detections");
		}

		drawMatLeft.copyTo(outLeft);
		drawMatRight.copyTo(outRight);

	}
	else {
		detectionsLeft.clear();
		detectionsRight.clear();
	}
}

vector<detected_object> ClassifierLens::Classify(InputArray in) {
	Mat analysisMat;
	in.copyTo(analysisMat);

	// TODO: Not sure this step is needed since customizing ofxDarknet to accept mats natively
	cv::cvtColor(analysisMat, analysisMat, CV_BGR2RGB);

	return darknet.yolo(analysisMat, threshold);
}

void ClassifierLens::DrawDetections(InputOutputArray mat, std::vector< detected_object > detections) {
	for (detected_object d : detections)
	{

		int lineThickness = 2;

		Scalar color = CV_RGB(0, 255, 0);

		cv::Rect rect = cv::Rect(d.rect.x, d.rect.y, d.rect.width, d.rect.height);
		if (doDownsampling) { // Rescale the rect to match original size if downsampling was performed
			rect = cv::Rect(d.rect.x / downSampleRatio, d.rect.y / downSampleRatio, d.rect.width / downSampleRatio, d.rect.height / downSampleRatio);
		}

		// Do the highlighting step before drawing anything else
		HighlightObjectInROI(mat, rect);

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
			if (!doDownsampling) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); // Push disabled style
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

			ImGui::Checkbox("Tracking", &useTracking);
			// Do tracker type dropdown
			int mode = (int)leftTracker.newTrackerType;
			ImGui::Combo("Mode", &mode, "KCF\0MIL\0TLD\0\0");

			// If updated, set the tracker types for the left/right trackers
			if (mode != (int)rightTracker.newTrackerType) {
				leftTracker.SetTrackerType((TrackerType)mode);
				rightTracker.SetTrackerType((TrackerType)mode);
			}

			ImGui::Spacing();
			ImGui::Spacing();

			// Draw the GUI panels for either the trackers or the raw detections panel
			if (useTracking) {
				leftTracker.DrawGUIPanel("Left");
				rightTracker.DrawGUIPanel("Right");
			}
			else {
				DrawDetectionsGUIPanel();
			}

		}

		// End GUI stuff
		if (!enabled) ImGui::PopStyleVar(); // Pop disabled style

		ImGui::End();
	}

}

void ClassifierLens::DrawDetectionsGUIPanel() {
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

/**
Highlighting step
http://creativemorphometrics.co.vu/blog/2014/08/05/automated-outlines-with-opencv-in-python/

*/
void ClassifierLens::HighlightObjectInROI(InputOutputArray mat, Rect2d roi) {
	TS_SCOPE("Highlight");

	Mat gray, prefiltered, canny;

	// Backward
	/*Mat srcRaw = mat.getMat();
	Mat src;
	srcRaw.copyTo(src(roi));*/

	// Copy in just the part of `mat` inside the ROI
	Mat srcRaw = mat.getMat();
	Mat src = srcRaw(roi);

	cvtColor(src, gray, COLOR_BGR2GRAY);

	// Preprocess
	//blur(gray, gray, cv::Size(7, 7));


	//cv::adaptiveThreshold(gray, gray, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 2);
	//cv::threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);

	Mat kernel = getStructuringElement(cv::MorphShapes::MORPH_RECT, cv::Size(5, 5));
	erode(gray, prefiltered, kernel, cv::Point(-1, -1), 1);

	int iterations = 1;
	morphologyEx(prefiltered, prefiltered, MORPH_OPEN, kernel, cv::Point(-1, -1), iterations);
	morphologyEx(prefiltered, prefiltered, MORPH_CLOSE, kernel, cv::Point(-1, -1), iterations);

	Canny(prefiltered, prefiltered, 30, 50);

	vector<vector<cv::Point>> contourData;
	vector<Vec4i> contourHierarchy;
	findContours(prefiltered, contourData, contourHierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	drawContours(prefiltered, contourData, -1, Scalar(255, 255, 255), 4);

	// Set the area the canny lines cover in the original image to the given color
	srcRaw(roi).setTo(Scalar(255, 0, 0), prefiltered);

	// Old drawing code (overlay, not masked outline)
	//Mat outColor;
	//cvtColor(prefiltered, outColor, COLOR_GRAY2BGR);
	//outColor.setTo(Scalar(255, 0, 0), prefiltered);
	//// Copy to original image in ROI
	//outColor.copyTo(srcRaw(roi));

}





