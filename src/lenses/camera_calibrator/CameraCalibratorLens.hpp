#pragma once

#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"

#include "lenses/LensCommon.hpp"
#include "gui/UsesGUI.hpp"

using namespace std;

class CameraCalibratorLens : public LensCommon, public UsesGUI {
public:
    CameraCalibratorLens();
    ~CameraCalibratorLens();
    
    void ProcessFrame(cv::InputArray in, cv::OutputArray out);
    
    void DrawGUI() override;
    
private:
    
};
