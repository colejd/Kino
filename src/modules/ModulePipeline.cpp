#include "ModulePipeline.hpp"

ModulePipeline::ModulePipeline() {

}

ModulePipeline::~ModulePipeline() {

}

void ModulePipeline::RegisterModule(ModuleCommon *module, bool preprocess) {
	if (preprocess) {
		preProcessorModules.push_back(module);
	}
	else {
		postProcessorModules.push_back(module);
	}
}

void ModulePipeline::DrawModuleGUIs() {
	// Draw each GUI if marked to show

	for (ModuleCommon* module : preProcessorModules) {
		if (module->showGUI) {
			module->PreGUI();
			module->DrawGUI();
			module->PostGUI();
		}
	}

	for (ModuleCommon* module : postProcessorModules) {
		if (module->showGUI) {
			module->PreGUI();
			module->DrawGUI();
			module->PostGUI();
		}
	}
}

void ModulePipeline::ProcessFrames(InputArray inLeft, InputArray inRight, OutputArray outLeft, OutputArray outRight) {
	Mat left = inLeft.getMat();
	Mat right = inRight.getMat();

	// Process all enabled utility modules 
	for (ModuleCommon* module : preProcessorModules) {
		if (module->enabled) {
			module->PreModule();
			module->ProcessFrames(left, right, left, right);
			module->PostModule();
		}
	}

	// Process all enabled user modules
	for (ModuleCommon* module : postProcessorModules) {
		if (module->enabled) {
			module->PreModule();
			module->ProcessFrames(left, right, left, right);
			module->PostModule();
		}
	}
}