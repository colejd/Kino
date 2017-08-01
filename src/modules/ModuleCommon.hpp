//
//  ModuleCommon.hpp
//  FaceOff
//
//  Created by Jonathan Cole on 11/3/15.
//
//

#ifndef ModuleCommon_h
#define ModuleCommon_h

#include <opencv2/opencv.hpp>
#include <gui/UsesGUI.hpp>

class ModuleCommon : public UsesGUI {
public:
	virtual ~ModuleCommon() {}
	virtual void ProcessFrames(cv::InputArray inLeft, cv::InputArray inRight, cv::OutputArray outLeft, cv::OutputArray outRight) = 0;
	virtual std::string GetName() = 0;

	void Enable() {
		enabled = true;
	}
	void Disable() {
		enabled = false;
	}
	const bool IsEnabled() {
		return enabled;
	}
	void ToggleEnabled() {
		enabled = !enabled;
	}
	bool enabled = false;

protected:

private:


};


#endif /* ModuleCommon_h */
