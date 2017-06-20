#include "ClassifierLens.hpp"

using namespace std;
using namespace cv;

//#include <Windows.h>

ClassifierLens::ClassifierLens() {

	//darknet.init("data/cfg/vgg-conv.cfg", "data/vgg-conv.weights");

	InitWithConfig(tinyYolo);

	//InitWithConfig(nightmare);

}

ClassifierLens::~ClassifierLens() {

}

void ClassifierLens::InitWithConfig(YoloConfig config) {
	this->config = config;
	string basePath = "data/darknet/";
	darknet.init(ofToDataPath(basePath + config.cfgFile),
				 ofToDataPath(basePath + config.weightFile),
				 config.namesList == "" ? "" : ofToDataPath(basePath + config.namesList));
}


void ClassifierLens::ProcessFrame(InputArray in, OutputArray out) {
	if (IsEnabled()) {

		// Will be drawn to, and eventually copied to the out mat.
		Mat drawingMat;
		in.copyTo(drawingMat);

		// Used for analysis.
		Mat analysisMat;

		// Downsample analysisMat if requested
		if (doDownsampling) {
			cv::resize(in, analysisMat, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
		}
		else {
			in.copyTo(analysisMat);
		}
		// Store the size of the downsampled image for debug later
		lastImageSize = Size { analysisMat.cols, analysisMat.rows };

		cv::cvtColor(analysisMat, analysisMat, CV_BGR2RGB);

		TS_START_NIF("YOLO");
		detections = darknet.yolo(analysisMat, threshold);
		TS_STOP_NIF("YOLO");

		TS_START_NIF("Draw Rects");
		for (detected_object d : detections)
		{

			int lineThickness = 4;

			cv::Rect rect = cv::Rect(d.rect.x, d.rect.y, d.rect.width, d.rect.height);
			if (doDownsampling) { // Rescale the rect to match original size if downsampling was performed
				rect = cv::Rect(d.rect.x / downSampleRatio, d.rect.y / downSampleRatio, d.rect.width / downSampleRatio, d.rect.height / downSampleRatio);
			}

			// Draw the bounding box
			// Random color: Scalar(rand() & 255, rand() & 255, rand() & 255)
			cv::rectangle(drawingMat, rect, Scalar(0, 255, 0), lineThickness, 8);

			// Draw the label in the bounding box
			int fontface = cv::FONT_HERSHEY_SIMPLEX;
			double scale = 1.0;
			int textThickness = 4;
			int baseline = 0;
			string label = d.label;

			cv::Size text = cv::getTextSize(label, fontface, scale, textThickness, &baseline);
			cv::Point origin = cv::Point(rect.x + lineThickness, rect.y + text.height + lineThickness);

			cv::rectangle(drawingMat, origin + cv::Point(0, baseline), origin + cv::Point(text.width, -text.height), CV_RGB(0, 0, 0), CV_FILLED);
			cv::putText(drawingMat, label, origin, fontface, scale, CV_RGB(255, 255, 255), textThickness, 8);

		}
		TS_STOP_NIF("Draw Rects");


		/*double alpha = blendAmt;
		if (!lastNightmare.empty) {
			drawingMat.copyTo(lastNightmare);
		}

		double beta = (1.0 - alpha);
		addWeighted(drawingMat, alpha, lastNightmare, beta, 0.0, drawingMat);*/

		////// blend last dream with cam
		//ofPixels p1 = nightmare.getPixels();
		//ofPixels p2 = img.getPixels();
		//ofPixels pix;
		//pix.allocate(p1.getWidth(), p2.getHeight(), 3);
		//for (int i = 0; i<pix.size(); i++) {
		//	pix[i] = blendAmt * p1[i] + (1.0 - blendAmt) * p2[i];
		//}

		//// dream
		//nightmare = darknet.nightmare(img.getPixelsRef(), max_layer, range, norm, 1, iters, octaves, rate, thresh);
		//
		//cv::Mat newMat = cv::Mat(nightmare.getHeight(), nightmare.getWidth(), CV_MAKETYPE(CV_8U, 3), nightmare.getPixelsRef().getData(), 0);
		//cvtColor(newMat, out, COLOR_BGR2RGB);


		drawingMat.copyTo(out);
	}
	else {
		detections.clear();
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
			ImGui::Text("Dimensions: (%i, %i)", lastImageSize.x, lastImageSize.y);

			// Info

			if (ImGui::TreeNode("Darknet Info")) {
				ImGui::Text("%-10s %s", "Config:", Paths::GetFileNameFromPath(config.cfgFile).c_str());
				ImGui::Text("%-10s %s", "Weights:", Paths::GetFileNameFromPath(config.weightFile).c_str());

				string namesList = Paths::GetFileNameFromPath(config.namesList);
				ImGui::Text("%-10s %s", "Names:", namesList == "" ? "None" : namesList.c_str());

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

				ImGuiListClipper clipper(detections.size(), ImGui::GetTextLineHeightWithSpacing());
				for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {

					detected_object d = detections[i];
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