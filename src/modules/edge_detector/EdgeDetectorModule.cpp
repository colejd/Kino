#include "EdgeDetectorModule.hpp"

EdgeDetectorModule::EdgeDetectorModule(){

}

EdgeDetectorModule::~EdgeDetectorModule(){
    
}

void EdgeDetectorModule::ProcessFrame(cv::InputArray in, cv::OutputArray out){

    if(!in.empty() && IsEnabled()){
        
		TS_START_NIF("Copy In");
        cv::Mat latestStep;
        cv::Mat finalFrame;
		cv::Mat gray;
		
        in.copyTo(latestStep);
		TS_STOP_NIF("Copy In");

		int originalWidth = latestStep.cols;
		int originalHeight = latestStep.rows;

		//Downsample
		if (doDownsampling) {
			cv::resize(latestStep, latestStep, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
		}
        
        //Condense the source image into a single channel for use with the Canny algorithm
		TS_START_NIF("Condense Channels");
        CondenseImage(latestStep, gray, currentChannelType);
		TS_STOP_NIF("Condense Channels");
        
        //threshold(edges, edges, 0, 255, THRESH_BINARY | THRESH_OTSU);
        
        //Blur the source image to reduce noise and texture details
		TS_START_NIF("Blur");
		//cv::Mat temp;
		//latestStep.copyTo(temp);
		////temp = latestStep.clone();
        BlurImage(gray, gray, currentBlurType);
		//temp.copyTo(latestStep);
		TS_STOP_NIF("Blur");
        
		//Super, super slow
        //fastNlMeansDenoising(latestStep, latestStep, 3, 7, 21);
        
        //Perform erosion and dilution if requested
        if(doErosionDilution){
			TS_START_NIF("Erode / Dilute");
            erode(gray, gray, UMat(), cv::Point(-1,-1), erosionIterations, BORDER_CONSTANT, morphologyDefaultBorderValue());
            dilate(gray, gray, UMat(), cv::Point(-1,-1), dilutionIterations, BORDER_CONSTANT, morphologyDefaultBorderValue());
			TS_STOP_NIF("Erode / Dilute");
		}
        
        //Perform edge detection
        
        //Canny step--------------------------------------
        //If the image has more than one color channel, then it wasn't condensed.
        //Divide it up and run Canny on each channel.
        //TODO: This should probably be run without any blurring beforehand.
		TS_START_NIF("Canny");
        if(gray.channels() > 1){
            std::vector<cv::Mat> channels;
            cv::split(gray, channels);
                
            //Separate the three color channels and perform Canny on each
            Canny(channels[0], channels[0], cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3);
            Canny(channels[1], channels[1], cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3);
            Canny(channels[2], channels[2], cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3);
                
            //Merge the three color channels into one image
            //merge(channels, canny_output);
            bitwise_or(channels[0], channels[1], channels[1]); //Merge 0 and 1 into 1
            bitwise_or(channels[1], channels[2], gray); //Merge 1 and 2 into result
        }
        else{
            Canny(gray, gray, cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3); //0, 30, 3
		}
		TS_STOP_NIF("Canny");
            
        //Contour step------------------------------------
        if(useContours){
			TS_START_NIF("Contour Detection");
            cv::Mat contourOutput;
            latestStep.copyTo(contourOutput);
            //ParallelContourDetector::DetectContoursParallel(latestStep, contourOutput, contourSubdivisions, lineThickness);
            contourOutput.copyTo(latestStep);
            //ParallelContourDetector::DetectContours(latestStep, latestStep, lineThickness);
			TS_STOP_NIF("Contour Detection");
        }

		//Restore original image size
		if (doDownsampling) {
			cv::resize(gray, gray, cv::Size(originalWidth, originalHeight), INTER_NEAREST);
		}
        
        //Output step-------------------------------------
		TS_START_NIF("Copy Output");
        if(showEdgesOnly){
            cvtColor(gray, finalFrame, COLOR_GRAY2BGR);
        }
        else{
            in.copyTo(finalFrame);
        }
		cv::Scalar finalColor = ColorToScalar(lineColor);
		if (useSmartLineColors) {
			//finalColor = cv::Scalar::all(255) - cv::mean(in);
			finalColor = cv::mean(in);
			averageColor = ImVec4(finalColor[2] / 255.0f, finalColor[1] / 255.0f, finalColor[0] / 255.0f, 1.0f);

			//Convert BGR to HSL
			int r = finalColor[2];
			int g = finalColor[1];
			int b = finalColor[0];

			int h = 0;
			int s = 0;
			int l = 0;

			RGBtoHSL(r, g, b, h, s, l);

			h = (h + 180) % 360; //Get complementary color
			s = 100;
			l = (l + 50) % 100; //Make it dark if it's light, make it light if it's dark (?)

			HSLtoRGB(h, s, l, r, g, b);

			finalColor = cv::Scalar(b, g, r);
			complementaryColor = ImVec4(finalColor[2] / 255.0f, finalColor[1] / 255.0f, finalColor[0] / 255.0f, 1.0f);

		}
		finalFrame.setTo(finalColor, gray);

        //Copy the result of all operations to the output frame.
        finalFrame.copyTo(out);
		TS_STOP_NIF("Copy Output");
        
		gray.release();
        latestStep.release();
        finalFrame.release();
        
    }
    //Directly draw the data from the frame
    else{
		TS_START_NIF("Copy Output");
        //if(in.empty()) printf("[EdgeDetectorModule] Frame is empty\n");
        //BlurImage(in, out, currentBlurType);
        //filterStylize(in, out);
        in.copyTo(out);
		TS_STOP_NIF("Copy Output");
    }

}

void EdgeDetectorModule::ProcessFrameOld(cv::InputArray in, cv::OutputArray out) {

	if (!in.empty() && IsEnabled()) {

		cv::Mat latestStep;
		cv::Mat finalFrame;

		TS_START_NIF("Copy In");
		in.copyTo(latestStep);
		TS_STOP_NIF("Copy In");

		int originalWidth = latestStep.cols;
		int originalHeight = latestStep.rows;

		cv::Scalar avgColor = cv::mean(in);

		//Downsample
		if (doDownsampling) {
			cv::resize(latestStep, latestStep, cv::Size(), downSampleRatio, downSampleRatio, INTER_NEAREST);
		}

		//Condense the source image into a single channel for use with the Canny algorithm
		TS_START_NIF("Condense Channels");
		CondenseImage(latestStep, latestStep, currentChannelType);
		TS_STOP_NIF("Condense Channels");

		//threshold(edges, edges, 0, 255, THRESH_BINARY | THRESH_OTSU);

		//Blur the source image to reduce noise and texture details
		TS_START_NIF("Blur");
		//cv::Mat temp;
		//latestStep.copyTo(temp);
		////temp = latestStep.clone();
		BlurImage(latestStep, latestStep, currentBlurType);
		//temp.copyTo(latestStep);
		TS_STOP_NIF("Blur");

		//Super, super slow
		//fastNlMeansDenoising(latestStep, latestStep, 3, 7, 21);

		//Perform erosion and dilution if requested
		if (doErosionDilution) {
			TS_START_NIF("Erode / Dilute");
			erode(latestStep, latestStep, UMat(), cv::Point(-1, -1), erosionIterations, BORDER_CONSTANT, morphologyDefaultBorderValue());
			dilate(latestStep, latestStep, UMat(), cv::Point(-1, -1), dilutionIterations, BORDER_CONSTANT, morphologyDefaultBorderValue());
			TS_STOP_NIF("Erode / Dilute");
		}

		//Perform edge detection

		//Canny step--------------------------------------
		//If the image has more than one color channel, then it wasn't condensed.
		//Divide it up and run Canny on each channel.
		//TODO: This should probably be run without any blurring beforehand.
		TS_START_NIF("Canny");
		if (latestStep.channels() > 1) {
			std::vector<cv::Mat> channels;
			cv::split(latestStep, channels);

			//Separate the three color channels and perform Canny on each
			Canny(channels[0], channels[0], cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3);
			Canny(channels[1], channels[1], cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3);
			Canny(channels[2], channels[2], cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3);

			//Merge the three color channels into one image
			//merge(channels, canny_output);
			bitwise_or(channels[0], channels[1], channels[1]); //Merge 0 and 1 into 1
			bitwise_or(channels[1], channels[2], latestStep); //Merge 1 and 2 into result
		}
		else {
			Canny(latestStep, latestStep, cannyThresholdLow, cannyThresholdLow * cannyThresholdRatio, 3); //0, 30, 3
		}
		TS_STOP_NIF("Canny");

		//Contour step------------------------------------
		if (useContours) {
			TS_START_NIF("Contour Detection");
			cv::Mat contourOutput;
			latestStep.copyTo(contourOutput);
			ParallelContourDetector::DetectContoursParallel(latestStep, contourOutput, contourSubdivisions, lineThickness);
			contourOutput.copyTo(latestStep);
			//ParallelContourDetector::DetectContours(latestStep, latestStep, lineThickness);
			TS_STOP_NIF("Contour Detection");
		}

		//Restore original image size
		if (doDownsampling) {
			cv::resize(latestStep, latestStep, cv::Size(originalWidth, originalHeight), INTER_NEAREST);
		}

		//Output step-------------------------------------
		TS_START_NIF("Copy Output");
		if (showEdgesOnly) {
			cvtColor(latestStep, finalFrame, COLOR_GRAY2BGR);
		}
		else {
			in.copyTo(finalFrame);
		}
		cv::Scalar finalColor = ColorToScalar(lineColor);
		if (useSmartLineColors) {
			//finalColor = cv::Scalar::all(255) - cv::mean(in);
			finalColor = cv::mean(in);
			averageColor = ImVec4(finalColor[2] / 255.0f, finalColor[1] / 255.0f, finalColor[0] / 255.0f, 1.0f);

			//Convert BGR to HSL
			int r = finalColor[2];
			int g = finalColor[1];
			int b = finalColor[0];

			int h = 0;
			int s = 0;
			int l = 0;

			RGBtoHSL(r, g, b, h, s, l);

			h = (h + 180) % 360; //Get complementary color
			s = 100;
			l = (l + 50) % 100; //Make it dark if it's light, make it light if it's dark (?)

			HSLtoRGB(h, s, l, r, g, b);

			finalColor = cv::Scalar(b, g, r);
			complementaryColor = ImVec4(finalColor[2] / 255.0f, finalColor[1] / 255.0f, finalColor[0] / 255.0f, 1.0f);

		}
		finalFrame.setTo(finalColor, latestStep);

		//Copy the result of all operations to the output frame.
		finalFrame.copyTo(out);
		TS_STOP_NIF("Copy Output");

		latestStep.release();
		finalFrame.release();

	}
	//Directly draw the data from the frame
	else {
		TS_START_NIF("Copy Output");
		//if(in.empty()) printf("[EdgeDetectorModule] Frame is empty\n");
		//BlurImage(in, out, currentBlurType);
		//filterStylize(in, out);
		in.copyTo(out);
		TS_STOP_NIF("Copy Output");
	}

}


/**
 Condenses the input image into one channel based on the ChannelType specified.
 */
void EdgeDetectorModule::CondenseImage(cv::InputArray in, cv::OutputArray out, int channelType = 0){
    if(channelType == ChannelType::GRAYSCALE){
        cvtColor(in, out, COLOR_BGR2GRAY);
    }
    else if(channelType == ChannelType::HUE){
		//http://stackoverflow.com/questions/29156091/opencv-edge-border-detection-based-on-color
        std::vector<cv::Mat> channels;
        cv::Mat hsv;
        cv::cvtColor( in, hsv, CV_BGR2HSV );
        cv::split(hsv, channels);
        channels[0].copyTo(out);
		//cv::Mat shiftedH;
		//out.copyTo(shiftedH);
		//int shift = 25; // in openCV hue values go from 0 to 180 (so have to be doubled to get to 0 .. 360) because of byte range from 0 to 255
		//for (int j = 0; j<shiftedH.rows; ++j)
		//	for (int i = 0; i<shiftedH.cols; ++i)
		//	{
		//		shiftedH.at<unsigned char>(j, i) = (shiftedH.at<unsigned char>(j, i) + shift) % 180;
		//	}
		//shiftedH.copyTo(out);
		cv::imshow("wow", out);
        hsv.release();
    }
    else if(channelType == ChannelType::COLOR){
        in.copyTo(out);
        //Do nothing
    }
}

/**
 Applies a blurring operation to the image based on blurType. Defaults to, uh,
 BlurType::DEFAULT, which is the regular OpenCV kernel blur function.
 */
void EdgeDetectorModule::BlurImage(cv::InputArray in, cv::OutputArray out, int blurType = 0){
    switch(blurType){
        default:
        case(BlurType::DEFAULT): //Homogeneous
            blur(in, out, cv::Size(7, 7));
            break;
        case(BlurType::GAUSSIAN):
            GaussianBlur(in, out, cv::Size(7,7), 1.5, 1.5);
            break;
        case(BlurType::NONE):
            break;
            
    }
}

void EdgeDetectorModule::DrawGUI() {
	if (showGUI) {
		ImGui::Begin("Edge Detector", &showGUI, ImGuiWindowFlags_AlwaysAutoResize);

		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		//if(enabled) ImGui::PushStyleVar(ImGuiCol_Text, ImColor::HSV(0, 0, 0.5));

		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push global disabled style

		ImGui::SliderInt("Low Threshold", &cannyThresholdLow, 0, 255);
		ShowHelpMarker("Set lower to look for more edges.");
		ImGui::SliderFloat("Ratio", &cannyThresholdRatio, 2.0, 3.0);
		ShowHelpMarker("Fine tune edge results.");
		ImGui::Checkbox("Use Smart Line Colors", &useSmartLineColors);
		if (!useSmartLineColors) {
			//ImGui::ColorEdit3("Line Color", (float*)&lineColor);
			ImGuiExtensions::ColorPicker("Line Color", lineColor);
			ShowHelpMarker("Click the color to show the editor.\nDouble click a field to type your own value.");
		}
		else {
			ImGui::ValueColor("Average", averageColor); 
			//ImGui::SameLine();
			ImGui::ValueColor("Complementary", complementaryColor);
		}
		ImGui::Checkbox("Edges Only", &showEdgesOnly);
		ShowHelpMarker("Show just the edges.");
		ImGui::Combo("Channel Type", &currentChannelType, channelTypes);

		//Contour settings
		ImGui::Spacing();
		ImGui::Text("Contour Settings");
		ImGui::Separator();
		ImGui::Checkbox("Use Contours", &useContours);
		ShowHelpMarker("Use contour detection to filter out short lines and noise in the edge data.");
		if (!useContours) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		ImGui::SliderInt("Subdivisions", &contourSubdivisions, 1, 16);
		ShowHelpMarker("Number of chunks the image is divided into for parallel processing.");
		ImGui::SliderInt("Thickness", &lineThickness, -1, 8);
		if (!useContours) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::Spacing();
		ImGui::Text("Image Tuning");
		ImGui::Separator();
		ImGui::Combo("Blur Type", &currentBlurType, blurTypes);
		ImGui::Checkbox("Erosion/Dilution", &doErosionDilution);
		ShowHelpMarker("Try to keep erosion and dilution at the same value.");
		if (!doErosionDilution) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		ImGui::SliderInt("Erosion Iterations", &erosionIterations, 0, 6);
		ShowHelpMarker("Morphological open operation; removes bright spots on a dark background.");
		ImGui::SliderInt("Dilution Iterations", &dilutionIterations, 0, 6);
		ShowHelpMarker("Morphologial close operation; removes dark spots on a bright background.");
		if (!doErosionDilution) ImGui::PopStyleVar(); //Pop disabled style

		ImGui::Checkbox("Downsample", &doDownsampling);
		if (!doDownsampling) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push disabled style
		ImGui::SliderFloat("Downsample Ratio", &downSampleRatio, 0.01f, 1.0f, "%.2f");
		ShowHelpMarker("Multiplier for decreasing the resolution of the processed image.");
		if (!doDownsampling) ImGui::PopStyleVar(); //Pop disabled style

		if (!enabled) ImGui::PopStyleVar(); //Pop global disabled style

		ImGui::End();

	}
}