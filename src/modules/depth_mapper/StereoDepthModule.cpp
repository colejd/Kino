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

		cv::cvtColor(imgDisparity8U, outRight, COLOR_GRAY2BGR);

	}
}

void StereoDepthModule::DrawGUI() {
	if (showGUI) {

		ImGui::Begin("Stereo Depth Module", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

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



		}
		//End main content
		if (!enabled) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::End();
	}
}
