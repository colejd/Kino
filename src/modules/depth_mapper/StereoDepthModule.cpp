#include "StereoDepthModule.hpp"

StereoDepthModule::StereoDepthModule() {
	// http://www.jayrambhia.com/blog/disparity-mpas
	sbm = StereoBM::create(numDisparities, blockSize);

	sbm->setPreFilterSize(5);
	sbm->setPreFilterCap(61);
	sbm->setMinDisparity(-39);
	sbm->setTextureThreshold(507);
	sbm->setUniquenessRatio(0);
	sbm->setSpeckleWindowSize(0);
	sbm->setSpeckleRange(8);
	sbm->setDisp12MaxDiff(1);

	//sbm->setMode(StereoSGBM::MODE_HH);
	//sbm->setP1(600);
	//sbm->setP2(2400);
}

StereoDepthModule::~StereoDepthModule() {

}

void StereoDepthModule::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	moduleCanRun = !inLeft.empty() && !inRight.empty();
	if (IsEnabled() && moduleCanRun) {
		TS_SCOPE("Stereo Depth Module");
		
		Mat imgDisparity16S = Mat(inLeft.size(), CV_16S);
		Mat imgDisparity8U = Mat(inRight.size(), CV_8UC1);

		Mat leftGray, rightGray;
		cvtColor(inLeft, leftGray, COLOR_BGR2GRAY);
		cvtColor(inRight, rightGray, COLOR_BGR2GRAY);

		//blur(leftGray, leftGray, cv::Size(7, 7));
		//blur(rightGray, rightGray, cv::Size(7, 7));

		sbm->compute(leftGray, rightGray, imgDisparity16S);

		//double minVal;
		//double maxVal;
		//minMaxLoc(imgDisparity16S, &minVal, &maxVal);
		//imgDisparity16S.convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));

		normalize(imgDisparity16S, imgDisparity8U, 0, 255, CV_MINMAX, CV_8U);

		//imshow("Disparity", imgDisparity16S);

		cv::cvtColor(imgDisparity8U, disparity, COLOR_GRAY2BGR);

	}
}

void StereoDepthModule::DrawGUI() {
	if (showGUI) {

		ImGui::Begin("Stereo Depth Module", &showGUI);

		if (!moduleCanRun) {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Cannot run without two camera captures enabled!");
			return;
		}

		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{
			ImGui::DragInt("Disparities", &numDisparities, 16, 16, 256);
			sbm->setNumDisparities(numDisparities);

			ImGui::DragInt("Block Size", &blockSize, 2, 5, 127);
			sbm->setBlockSize(blockSize);

			// Draw a preview of the window if it exists
			if (!disparity.empty()) {
				ImGui::Spacing();

				ImGui::Text("Output");

				float baseSize = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2); // Determines the width of the image. Height is scaled.
				double aspect = (double)disparity.rows / (double)disparity.cols;
				ImVec2 previewSize(baseSize, baseSize * aspect);

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::BeginChild("Disparity Preview", previewSize, true, ImGuiWindowFlags_ChildWindowAutoFitY);
				{

					GLuint textureID;
					glGenTextures(1, &textureID);
					glBindTexture(GL_TEXTURE_2D, textureID);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

					GLint internalFormat = GL_RGB;
					GLint imageFormat = GL_BGR;
					glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, disparity.cols, disparity.rows, 0, imageFormat, GL_UNSIGNED_BYTE, disparity.ptr());
					glGenerateMipmap(GL_TEXTURE_2D);

					float width = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2);
					float height = ImGui::GetWindowHeight() - (ImGui::GetStyle().WindowPadding.y * 2);
					ImGui::Image((ImTextureID)textureID, ImVec2(width, height));

				}
				ImGui::EndChild();
				ImGui::PopStyleVar();

			}


		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}
