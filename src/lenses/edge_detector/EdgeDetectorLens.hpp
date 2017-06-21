#pragma once

#include <stdio.h>
#include <string>

#include "opencv2/opencv.hpp"
#include <opencv2/core/ocl.hpp>

#include "lenses/LensCommon.hpp"

#include "gui/UsesGUI.hpp"
#include "gui/thirdparty/ImGuiColorPicker.hpp"
#include "config/ConfigHandler.hpp"
#include "ParallelContourDetector.hpp"

#include "ofxImGui.h"
#include "ofxTimeMeasurements.h"

#include "util/ColorConversion.hpp"


using namespace std;
using namespace cv;

class EdgeDetectorLens : public LensCommon, public UsesGUI {
public:
    EdgeDetectorLens();
    ~EdgeDetectorLens();
    
    void DrawGUI() override;
    
    void ProcessFrame(cv::InputArray in, cv::OutputArray out);

	void ProcessFrameOld(cv::InputArray in, cv::OutputArray out);
    
    enum ChannelType{
        GRAYSCALE, //!< Use Greyscale conversion with Canny
        HUE, //!< (huehuehuehue) Use Hue channel with Canny
        COLOR, //!< Use R, G, and B channels with Canny then combine
    };
    //std::vector<string> channelTypeVec {"Grayscale", "Hue", "Color"};
	char *channelTypes = "Grayscale\0Hue\0Color\0\0";
    
    enum BlurType{
        DEFAULT,
        GAUSSIAN,
        NONE
    };
    //std::vector<string> blurTypeVec {"Default", "Gaussian", "Adaptive Manifold", "DTFilter", "None"};
	char *blurTypes = "Default\0Gaussian\0None\0\0";
    
    void BlurImage(cv::InputArray in, cv::OutputArray out, int blurType);
    void CondenseImage(cv::InputArray in, cv::OutputArray out, int channelType);
    
	ImVec4 averageColor = ImVec4(1.0f, 1.0f, 1.0, 1.0f);
	ImVec4 complementaryColor = ImVec4(1.0f, 1.0f, 1.0, 1.0f);
    
    
private:
    int currentChannelType = ChannelType::GRAYSCALE;
    int currentBlurType = BlurType::DEFAULT;
    //The final image that will be shown
    Mat finalMat;
    
    bool drawEdges = true;
    bool doStructuredEdgeForests = false;
    int cannyThresholdLow = 30; //0
    int cannyThresholdHigh = 50; //50
    float cannyThresholdRatio = 2.0;
    
    bool useContours = false;
    
    bool doErosionDilution = false;
    int erosionIterations = 1;
    int dilutionIterations = 1;
    
    int contourSubdivisions = 4;
    
    int lineThickness = 2;
	float minContourSize = 0.0;
    
    bool showEdgesOnly = false;
    
    ImVec4 lineColor = ImVec4(1.0f, 1.0f, 0, 1.0f);
    
    static cv::Scalar ColorToScalar(const ImVec4 c){
        return cv::Scalar(c.z * 255.0f, c.y * 255.0f, c.x * 255.0f);
    }
    
    double sigma_s = 24.0;
    double sigma_r = 0.2;
    
    int sigmaColor = 25;
    int sigmaSpatial = 10;
    int edgesGamma = 100;

	bool doDownsampling = false;
	float downSampleRatio = 0.5f;

	bool useSmartLineColors = false;
};
