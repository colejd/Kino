#include "DeepDreamLens.hpp"

using namespace std;
using namespace cv;

DeepDreamLens::DeepDreamLens() {
}

DeepDreamLens::~DeepDreamLens() {
}

void DeepDreamLens::Initialize() {
	darknet.init(ofToDataPath("data/darknet/cfg/vgg-conv.cfg"), ofToDataPath("data/darknet/vgg-conv.weights"));
	initialized = true;
}

void DeepDreamLens::ProcessFrame(InputArray in, OutputArray out) {
	if (IsEnabled()) {
		TS_START_NIF("DeepDream Lens");

		// Initialize on-demand
		if (!initialized) {
			Initialize();
		}

		Mat drawingMat;
		in.copyTo(drawingMat);

		// Used for analysis.
		Mat analysisMat;
		in.copyTo(analysisMat);

		// Convert the mat to an ofImage for processing
		TS_START_NIF("Preparation");
		/*ofImage img;
		img.allocate(analysisMat.cols, analysisMat.rows, OF_IMAGE_COLOR);
		img.getTexture().loadData(analysisMat.data, analysisMat.cols, analysisMat.rows, GL_BGR_EXT);*/
		ofImage img;
		img.allocate(analysisMat.cols, analysisMat.rows, OF_IMAGE_COLOR);

		cv::Mat converted;
		cvtColor(analysisMat, converted, COLOR_BGR2RGB);
		img.setFromPixels(converted.data, converted.cols, converted.rows, OF_IMAGE_COLOR);
		TS_STOP_NIF("Preparation");

		TS_START_NIF("Blend");
		//// blend last dream with cam
		ofPixels p1 = nightmare.getPixels();
		ofPixels p2 = img.getPixels();
		ofPixels pix;
		pix.allocate(p1.getWidth(), p2.getHeight(), 3);
		for (int i = 0; i < pix.size(); i++) {
			pix[i] = blendAmt * p1[i] + (1.0 - blendAmt) * p2[i];
		}
		TS_STOP_NIF("Blend");

		TS_START_NIF("Dream");
		// dream
		nightmare = darknet.nightmare(img.getPixelsRef(), max_layer, range, norm, rounds, iters, octaves, rate, thresh);
		TS_STOP_NIF("Dream");

		TS_START_NIF("Output");
		cv::Mat newMat = cv::Mat(nightmare.getHeight(), nightmare.getWidth(), CV_MAKETYPE(CV_8U, 3), nightmare.getPixelsRef().getData(), 0);
		cvtColor(newMat, out, COLOR_RGB2BGR);
		TS_STOP_NIF("Output");


		TS_STOP_NIF("DeepDream Lens");
	}

}

void DeepDreamLens::DrawGUI() {
	if (showGUI) {
		ImGui::Begin("DeepDream", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		//if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		{
			ImGui::SliderInt("max_layer", &max_layer, 1, 13);
			ImGui::SliderInt("iters", &iters, 1, 20);
			ImGui::SliderInt("octaves", &octaves, 1, 8);
			ImGui::SliderFloat("thresh", &thresh, 0.0, 1.0);
			ImGui::SliderInt("range", &range, 1, 10);
			ImGui::SliderInt("norm", &norm, 1, 10);
			ImGui::SliderFloat("rate", &rate, 0.0, 0.1);
			ImGui::SliderFloat("blendAmt", &blendAmt, 0.0, 1.0);
			ImGui::SliderInt("rounds", &rounds, 1, 10);

			if (ImGui::Button("Load Preset (massive framerate drop)")) {
				max_layer = 13;
				range = 3;
				norm = 1;
				rounds = 4;
				iters = 20;
				octaves = 4;
				rate = 0.01;
				thresh = 1.0;
			}

		}
		//if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}