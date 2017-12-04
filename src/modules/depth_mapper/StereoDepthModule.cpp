#include "StereoDepthModule.hpp"

StereoDepthModule::StereoDepthModule() {
	// http://www.jayrambhia.com/blog/disparity-mpas
	leftBM = StereoBM::create(numDisparities, blockSize);
	/*leftBM->setPreFilterSize(5);
	leftBM->setPreFilterCap(61);
	leftBM->setMinDisparity(-39);
	leftBM->setTextureThreshold(507);
	leftBM->setUniquenessRatio(0);
	leftBM->setSpeckleWindowSize(0);
	leftBM->setSpeckleRange(8);
	leftBM->setDisp12MaxDiff(1);*/

	// https://github.com/bnascimento/opencv-2.4/blob/master/samples/python2/stereo_match.py
	/*int window_size = 3;
	int min_disp = 16;
	int num_disp = 112 - min_disp;
	leftBM = StereoSGBM::create(min_disp, num_disp, window_size);
	leftBM->setMinDisparity(min_disp);
	leftBM->setNumDisparities(num_disp);
	leftBM->setBlockSize(window_size);
	leftBM->setUniquenessRatio(10);
	leftBM->setSpeckleWindowSize(100);
	leftBM->setSpeckleRange(32);
	leftBM->setDisp12MaxDiff(1);
	leftBM->setP1(8 * 3 * (window_size * window_size));
	leftBM->setP2(32 * 3 * (window_size * window_size));
*/
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

	//sbm->setMode(StereoSGBM::MODE_HH);

	rightBM = createRightMatcher(leftBM);

	wls_filter = createDisparityWLSFilter(leftBM);

}

StereoDepthModule::~StereoDepthModule() {

}

void StereoDepthModule::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	moduleCanRun = !inLeft.empty() && !inRight.empty();
	if (!moduleCanRun) {
		return;
	}
		
	Mat imgDisparity16S_Left, imgDisparity16S_Right;// = Mat(inLeft.size(), CV_16S);

	UMat leftGrayRaw, rightGrayRaw;
	cvtColor(inLeft, leftGrayRaw, COLOR_BGR2GRAY);
	cvtColor(inRight, rightGrayRaw, COLOR_BGR2GRAY);

	int originalWidth = leftGrayRaw.cols;
	int originalHeight = rightGrayRaw.rows;

	UMat leftGray, rightGray;
	int max_disp = numDisparities;
	int wsize = blockSize;
	// Downsample analysisMat if requested
	if (doDownsampling) {
		TS_SCOPE("Downsample");
		max_disp /= 2;
		if (max_disp % 16 != 0)
			max_disp += 16 - (max_disp % 16);

		wsize = 7;

		cv::resize(leftGrayRaw, leftGray, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
		cv::resize(rightGrayRaw, rightGray, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
	}
	else {
		leftGrayRaw.copyTo(leftGray);
		rightGrayRaw.copyTo(rightGray);
	}

	leftBM->setNumDisparities(max_disp);
	leftBM->setBlockSize(wsize);
	rightBM->setNumDisparities(max_disp);
	rightBM->setBlockSize(wsize);


	//blur(leftGray, leftGray, cv::Size(7, 7));
	//blur(rightGray, rightGray, cv::Size(7, 7));

	TS_START_NIF("BM Left");
	leftBM->compute(leftGray, rightGray, imgDisparity16S_Left);
	TS_STOP_NIF("BM Left");

	// Compress and normalize to CV_8U
	// https://stackoverflow.com/questions/28959440/how-to-access-the-disparity-value-in-opencv
	//normalize(imgDisparity16S, imgDisparity8U, 0, 255, NORM_MINMAX, CV_8U);

	//imshow("Disparity", imgDisparity16S);

	cv::Mat disparity16S;

	if (filter) {
		TS_START_NIF("BM Right");
		rightBM->compute(rightGray, leftGray, imgDisparity16S_Right);
		TS_STOP_NIF("BM Right");

		TS_START_NIF("Filter");
		wls_filter->setLambda(wls_lambda);
		wls_filter->setSigmaColor(wls_sigma);
		wls_filter->filter(imgDisparity16S_Left, inLeft, disparity16S, imgDisparity16S_Right);
		TS_STOP_NIF("Filter");
	}
	else {
		disparity16S = imgDisparity16S_Left;
	}

	getDisparityVis(disparity16S, disparity);

	// Optional normalization step
	TS_START_NIF("Normalize");
	normalize(disparity, disparity, 0, 255, NORM_MINMAX, CV_8U);
	TS_STOP_NIF("Normalize");

	// Convert and upscale
	cv::cvtColor(disparity, disparity, COLOR_GRAY2BGR);

	// No need to actually upsample yet
	/*if (doDownsampling) {
		TS_SCOPE("Upsample");
		Mat copy = disparity;
		cv::resize(copy, disparity, cv::Size(originalWidth, originalHeight), INTER_NEAREST);
	}*/

}

void StereoDepthModule::DrawGUI() {

	if (!moduleCanRun) {
		ImGui::TextColored(ImColor(255, 0, 0), "Cannot run without two camera captures enabled!");
		return;
	}

	ImGui::DragInt("Disparities", &numDisparities, 16, 16, 256);
	leftBM->setNumDisparities(numDisparities);
	rightBM->setNumDisparities(numDisparities);

	ImGui::DragInt("Block Size", &blockSize, 2, 5, 21);
	leftBM->setBlockSize(blockSize);
	rightBM->setBlockSize(blockSize);

	ImGui::Checkbox("Filter", &filter);

	ImGui::Checkbox("Downsample", &doDownsampling);
	if (!doDownsampling) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
	ImGui::SliderFloat("Downsample Ratio", &downSampleRatio, 0.01f, 1.0f, "%.2f");
	ShowHelpMarker("Multiplier for decreasing the resolution of the processed image.");
	if (!doDownsampling) ImGui::PopStyleVar(); //Pop disabled style

	// Draw a preview of the window if it exists
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	DrawImguiMat(disparity, "Disparity");

}

void StereoDepthModule::DrawImguiMat(InputArray in, string id) {
	ImGui::Text(id.c_str());

	if (!in.empty()) {

		// If there is no reserved GL texture for this mat (by ID), then create one.
		if (textureMap.find(id) == textureMap.end()) {
			// Reserve GL texture ID for disparity mat
			glGenTextures(1, &textureMap[id]);
		}

		Mat mat;
		in.copyTo(mat);

		float baseSize = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2); // Determines the width of the image. Height is scaled.
		double aspect = (double)mat.rows / (double)mat.cols;
		ImVec2 previewSize(baseSize, baseSize * aspect);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::BeginChild("Disparity Preview", previewSize, true);
		{
			// Update OpenGL image data
			glBindTexture(GL_TEXTURE_2D, textureMap[id]);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

			GLint internalFormat = GL_RGB; // Format of the OpenGL texture
			GLint imageFormat = GL_BGR; // Format of the input data (OpenCV Mat here)
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mat.cols, mat.rows, 0, imageFormat, GL_UNSIGNED_BYTE, mat.ptr());
			glGenerateMipmap(GL_TEXTURE_2D);

			// Draw image to fill container (up to container to maintain aspect)
			float width = ImGui::GetWindowWidth() - (ImGui::GetStyle().WindowPadding.x * 2);
			float height = ImGui::GetWindowHeight() - (ImGui::GetStyle().WindowPadding.y * 2);
			ImGui::Image((ImTextureID)textureMap[id], ImVec2(width, height));

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
