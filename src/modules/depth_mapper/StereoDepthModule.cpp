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

	// https://github.com/bnascimento/opencv-2.4/blob/master/samples/python2/stereo_match.py
	/*int window_size = 3;
	int min_disp = 16;
	int num_disp = 112 - min_disp;
	sbm = StereoSGBM::create(min_disp, num_disp, window_size);
	sbm->setMinDisparity(min_disp);
	sbm->setNumDisparities(num_disp);
	sbm->setBlockSize(window_size);
	sbm->setUniquenessRatio(10);
	sbm->setSpeckleWindowSize(100);
	sbm->setSpeckleRange(32);
	sbm->setDisp12MaxDiff(1);
	sbm->setP1(8 * 3 * (window_size * window_size));
	sbm->setP2(32 * 3 * (window_size * window_size));*/

	//// http://www.jayrambhia.com/blog/disparity-mpas
	//sbm = StereoSGBM::create(0, 0, 0);
	//sbm->setPreFilterCap(4);
	//sbm->setMinDisparity(-64);
	//sbm->setNumDisparities(192);
	//sbm->setBlockSize(5);
	//sbm->setUniquenessRatio(1);
	//sbm->setSpeckleWindowSize(150);
	//sbm->setSpeckleRange(2);
	//sbm->setDisp12MaxDiff(10);
	//sbm->setP1(600);
	//sbm->setP2(2400);



	// Reserve GL texture ID for disparity mat
	glGenTextures(1, &previewTextureID);

}

StereoDepthModule::~StereoDepthModule() {

}

void StereoDepthModule::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	moduleCanRun = !inLeft.empty() && !inRight.empty();
	if (IsEnabled() && moduleCanRun) {
		TS_SCOPE("Stereo Depth Module");
		
		Mat imgDisparity16S;// = Mat(inLeft.size(), CV_16S);
		Mat imgDisparity8U;// = Mat(inRight.size(), CV_8UC1);

		Mat leftGrayRaw, rightGrayRaw;
		cvtColor(inLeft, leftGrayRaw, COLOR_BGR2GRAY);
		cvtColor(inRight, rightGrayRaw, COLOR_BGR2GRAY);

		int originalWidth = leftGrayRaw.cols;
		int originalHeight = rightGrayRaw.rows;

		Mat leftGray, rightGray;
		// Downsample analysisMat if requested
		if (doDownsampling) {
			cv::resize(leftGrayRaw, leftGray, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
			cv::resize(rightGrayRaw, rightGray, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
		}
		else {
			leftGrayRaw.copyTo(leftGray);
			rightGrayRaw.copyTo(rightGray);
		}




		//blur(leftGray, leftGray, cv::Size(7, 7));
		//blur(rightGray, rightGray, cv::Size(7, 7));

		sbm->compute(leftGray, rightGray, imgDisparity16S);

		//Restore original image size
		if (doDownsampling) {
			Mat copy = imgDisparity16S;
			cv::resize(copy, imgDisparity16S, cv::Size(originalWidth, originalHeight), INTER_NEAREST);
		}

		/*double minVal;
		double maxVal;
		minMaxLoc(imgDisparity16S, &minVal, &maxVal);
		imgDisparity16S.convertTo(imgDisparity8U, CV_8UC1, 255 / (maxVal - minVal));*/

		// Compress and normalize to CV_8U
		// https://stackoverflow.com/questions/28959440/how-to-access-the-disparity-value-in-opencv
		normalize(imgDisparity16S, imgDisparity8U, 0, 255, NORM_MINMAX, CV_8U);

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
		ImGui::Spacing();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		//Begin main content
		{
			ImGui::DragInt("Disparities", &numDisparities, 16, 16, 256);
			sbm->setNumDisparities(numDisparities);

			ImGui::DragInt("Block Size", &blockSize, 2, 5, 21);
			sbm->setBlockSize(blockSize);

			ImGui::Checkbox("Downsample", &doDownsampling);
			if (!doDownsampling) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
			ImGui::SliderFloat("Downsample Ratio", &downSampleRatio, 0.01f, 1.0f, "%.2f");
			ShowHelpMarker("Multiplier for decreasing the resolution of the processed image.");
			if (!doDownsampling) ImGui::PopStyleVar(); //Pop disabled style

			// Draw a preview of the window if it exists
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Text("Output");

			if (!disparity.empty()) {

				float baseSize = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2); // Determines the width of the image. Height is scaled.
				double aspect = (double)disparity.rows / (double)disparity.cols;
				ImVec2 previewSize(baseSize, baseSize * aspect);

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
				ImGui::BeginChild("Disparity Preview", previewSize, true);
				{
					// Update OpenGL image data
					glBindTexture(GL_TEXTURE_2D, previewTextureID);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

					GLint internalFormat = GL_RGB; // Format of the OpenGL texture
					GLint imageFormat = GL_BGR; // Format of the input data (OpenCV Mat here)
					glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, disparity.cols, disparity.rows, 0, imageFormat, GL_UNSIGNED_BYTE, disparity.ptr());
					glGenerateMipmap(GL_TEXTURE_2D);

					// Draw image to fill container (up to container to maintain aspect)
					float width = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2);
					float height = ImGui::GetWindowHeight() - (ImGui::GetStyle().WindowPadding.y * 2);
					ImGui::Image((ImTextureID)previewTextureID, ImVec2(width, height));

				}
				ImGui::EndChild();
				ImGui::PopStyleVar();

			}
			else {
				// Draw fake output window with small size
				int width = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2);
				ImGui::BeginChild("Disparity Preview", ImVec2(std::min(320, width), 240), true);

				ImGui::EndChild();
			}


		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}
