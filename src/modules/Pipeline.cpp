#include "Pipeline.hpp"

Pipeline::Pipeline() {

}

Pipeline::~Pipeline() {

}

void Pipeline::RegisterModule(ModuleCommon *module, bool preprocess) {
	if (preprocess) {
		preProcessorModules.push_back(module);
	}
	else {
		postProcessorModules.push_back(module);
	}
}

void Pipeline::DrawModuleGUIs() {
	for (ModuleCommon* module : preProcessorModules) {
		module->DrawGUI();
	}

	for (ModuleCommon* module : postProcessorModules) {
		module->DrawGUI();
	}
}

void Pipeline::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	Mat left = inLeft.getMat();
	Mat right = inRight.getMat();

	for (ModuleCommon* module : preProcessorModules) {
		module->ProcessFrames(left, right, left, right);
	}

	for (ModuleCommon* module : postProcessorModules) {
		module->ProcessFrames(left, right, left, right);
	}
}