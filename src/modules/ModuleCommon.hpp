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
#include "ofxTimeMeasurements.h"

class ModuleCommon : public UsesGUI {
public:

	virtual ~ModuleCommon() {}

	// A module must always override the following two methods and DrawGUI from the UsesGUI class.
	virtual void ProcessFrames(cv::InputArray inLeft, cv::InputArray inRight, cv::OutputArray outLeft, cv::OutputArray outRight) = 0;
	virtual std::string GetName() = 0;

	/*void Enable() {
		enabled = true;
	}
	void Disable() {
		enabled = false;
	}
	const bool IsEnabled() {
		return enabled;
	}*/
	void ToggleEnabled() {
		enabled = !enabled;
	}
	bool enabled = false;

	virtual void PreGUI() {
		ImGui::Begin(GetName().c_str(), &showGUI, ImGuiWindowFlags_AlwaysAutoResize);
		ImGui::Checkbox("Enabled", &enabled);
		ImGui::Separator();
		if (!enabled) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.2); //Push global disabled style
	}
	virtual void PostGUI() {
		if (!enabled) ImGui::PopStyleVar(); //Pop global disabled style
		ImGui::End();
	}
	virtual void PreModule() {
		TS_START_NIF(GetName());
	}
	virtual void PostModule(){
		TS_STOP_NIF(GetName());
	}

protected:

private:

};


#endif /* ModuleCommon_h */
