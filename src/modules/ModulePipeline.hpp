#pragma once

#include <opencv2/opencv.hpp>
#include "KinoGlobals.hpp"
#include "ModuleCommon.hpp"

#include "ofxTimeMeasurements.h"

using namespace std;
using namespace cv;

/**

Centralized collection for objects deriving from ModuleCommon. Gives you single
commands for drawing module GUIs and processing frames through them.

*/
class ModulePipeline {
public:
	ModulePipeline();
	~ModulePipeline();

	void RegisterModule(ModuleCommon *module, bool preprocess = false);
	void DrawModuleGUIs();
	void ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight);

	vector<ModuleCommon*> preProcessorModules;
	vector<ModuleCommon*> postProcessorModules;

private:

};