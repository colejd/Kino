#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"

#include "ofMain.h"

#include "lenses/LensCommon.hpp"
#include "gui/UsesGUI.hpp"
#include "ofxTimeMeasurements.h"

using namespace std;

class FaceDetectorLens : public LensCommon, public UsesGUI {
public:
    FaceDetectorLens();
    ~FaceDetectorLens();
    
    void ProcessFrame(cv::InputArray in, cv::OutputArray out);
    
    void DrawGUI() override;
    
private:
    cv::CascadeClassifier face_cascade;
    cv::CascadeClassifier eyes_cascade;
    
    float imageScale = 0.125;
    
};
