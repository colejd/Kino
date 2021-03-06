#include "ofApp.h"


#include "ofxTimeMeasurements.h"

using namespace cv;

ofApp::ofApp()
{
}

ofApp::~ofApp()
{
	delete core;
	delete compositor;
}

//--------------------------------------------------------------
void ofApp::setup() {
	//Configure OpenFrameworks
	//Disable default escape key shortcut so our shortcut will work
	//ofSetEscapeQuitsApp(false);

	//Configure timing macros
	TIME_SAMPLE_SET_FRAMERATE(60.0f); //Specify a target framerate
	//TIME_SAMPLE_GET_INSTANCE()->setUiScale(1.5); //Do this before setting font
	TIME_SAMPLE_GET_INSTANCE()->drawUiWithFontStash("fonts/ProggyClean.ttf"); //Relative to data folder
	TIME_SAMPLE_GET_INSTANCE()->setUIActivationKey('~');
	//TIME_SAMPLE_GET_INSTANCE()->setHighlightColor(ofColor::red);
	TIME_SAMPLE_SET_AVERAGE_RATE(0.01); //Every new sample effects 1% of the value shown
	TIME_SAMPLE_GET_INSTANCE()->setEnabled(true); //On by default

	//Configure the GUI
	//Scale up the UI
	float configGUIScale = ConfigHandler::GetValue("GUI_SCALE", 1.0f).asFloat();
	//ImGui::GetIO().DisplayFramebufferScale = ImVec2(configGUIScale, configGUIScale);
	gui.setup();
	SetGUITheme();

	//ofSetFrameRate(120);
	ofSetVerticalSync(ConfigHandler::GetValue("USE_VSYNC", false).asBool());

	//Init class members
	core = new KinoCore();
	compositor = new ImageCompositor(ofGetWindowWidth(), ofGetWindowHeight()); //Set to whatever you need for your cameras to fit side by side //1280, 480

	ToggleTimingWindow(); //Turn off the timing window for now

	demoMode = ConfigHandler::GetValue("DEMO_SETTINGS.ACTIVE", false).asBool();

	// Load the image used in the help screen
	iconTextureID = (ImTextureID) gui.loadImage(ofToDataPath("KinoLogo_256.png"));

}

void ofApp::update() {
	//Update opencv stuff
	TS_START_NIF("core");
	core->Update();
	compositor->Update();
	TS_STOP_NIF("core");
	coreFps = TIME_SAMPLE_GET_LAST_DURATION("core");
}

void ofApp::draw() {
	TSGL_START("Draw");
	if (demoMode) {
		// Draw leftmat fullscreen
		compositor->DrawMatFullscreen(core->leftMat);
	}
	else {
		// Draw leftmat / rightmat side by side
		compositor->DrawMatsToFbo(core->leftMat, core->rightMat);
		compositor->DrawWindowFbo(); //Draws topmost FBO
	}

	//Draw gui last
	DrawAllGUI();
	TSGL_STOP("Draw");
}

/*
Entry point for all GUI drawing in the program.
*/
void ofApp::DrawAllGUI() {

	//Draw performance graph for eye
	if (showPerformanceGraph) {
		TS_START_NIF("PerfGraph");
		fpsGraph.fillGraph = true;
		//fpsGraph.SetPosition(ofGetWindowWidth() - 70, ofGetWindowHeight() - 15);
		fpsGraph.Enqueue(ImGui::GetIO().Framerate);
		//fpsGraph.Draw(ofGetWindowWidth() - 120, 20, 120, 90); //Top right
		fpsGraph.Draw(10, ofGetWindowHeight() - 100, 120, 90); //Bottom left
		TS_STOP_NIF("PerfGraph");
	}

	//Draw the ImGui toolbar and windows
	TS_START_NIF("ImGui Draw");
	gui.begin();
	{
		//Draw the GUI for this class
		DrawGUI();
		//Draw the core GUI and its modules' GUIs
		core->DrawAllGUIs();
		//Draw the compositor's GUI
		compositor->DrawGUI();
	}
	gui.end();
	TS_STOP_NIF("ImGui Draw");
}

void ofApp::DrawGUI() {

	//ImGui::ShowTestWindow();

	// Draw the menu bar
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("Kino")) {
		ImGui::MenuItem("Pause", "Alt + P", &core->pauseCaptureUpdates);
		if (ImGui::MenuItem("Fullscreen", "Alt + F")) {
			ofToggleFullscreen();
		}
		if (ImGui::MenuItem("Take Screenshot", "F11")) {
			TakeScreenshot();
		}
		ImGui::MenuItem("Help", "F1", &showHelp);
		if (ImGui::MenuItem("Quit", "ESC")) {
			ofExit(EXIT_SUCCESS);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Modules")) {
		// Draw an option for each module, starting with preprocessing modules
		ImGui::TextColored(ImColor(255, 255, 0), "Preprocess");
		for (ModuleCommon* module : core->modulePipeline.preProcessorModules) {
			ImGui::MenuItem(module->GetName().c_str(), nullptr, &(module->showGUI));
		}

		ImGui::Separator();
		ImGui::TextColored(ImColor(255, 255, 0), "Postprocess");
		for (ModuleCommon* module : core->modulePipeline.postProcessorModules) {
			ImGui::MenuItem(module->GetName().c_str(), nullptr, &(module->showGUI));
		}

		ImGui::EndMenu();
	}

	if (!demoMode && ImGui::BeginMenu("HMD")) {
		if (!demoMode) {
			ImGui::MenuItem("Swap Cameras", "Alt + S", &(core->swapSides));
		}
		ImGui::MenuItem("Distortion", "Alt + D", &(compositor->doDistortion));

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View")) {
		ImGui::MenuItem("General Settings", nullptr, &showGeneralSettingsWindow);
		if (!demoMode) ImGui::MenuItem("Compositor Settings", nullptr, &(compositor->showGUI));
		ImGui::MenuItem("Log", "Alt + L", &showLog);
		ImGui::MenuItem("FPS Graph", nullptr, &showPerformanceGraph);
		if (ImGui::MenuItem("Performance", "~")) {
			ToggleTimingWindow();
		}
		ImGui::EndMenu();
	}

	// Draw core timing
	//ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	//if (coreFps > frameTarget) textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); //Set red if over max frame time
	//else if (coreFps > frameTarget / 2.0f) textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); //Set yellow if halfway to max frame time
	//ImGui::SameLine(ImGui::GetWindowWidth() - 160); ImGui::TextColored(textColor, "core: %.1f ms", coreFps);

	// Draw capture FPS (not actual camera FPS, just frequency of the core polling the capture)
	//ImGui::SameLine(ImGui::GetWindowWidth() - 500); ImGui::Text("Cameras: %4.0f %4.0f", core->framebuffer[LEFT_ID].avgFPS, core->framebuffer[RIGHT_ID].avgFPS);


#ifdef _DEBUG
	ImGui::SameLine(ImGui::GetWindowWidth() - 250);
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Debug Build");
#endif

	// Draw GPU usage percentage
	ImGui::SameLine(ImGui::GetWindowWidth() - 160);
	float gpuUsage = GPUMonitor::GetInstance().GetMemoryUsagePercent();
	ImVec4 gpuTextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	if (gpuUsage > 0.5) gpuTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	else if (gpuUsage > 0.8) gpuTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	ImGui::TextColored(gpuTextColor, "GPU: %3.1f%%", gpuUsage * 100.0);

	// Draw application fps
	ImGui::SameLine(ImGui::GetWindowWidth() - 70);
	//char buff[32];
	//snprintf(buff, sizeof(buff), " %3.0f FPS ", ImGui::GetIO().Framerate);
	//if (ImGui::Button(buff)) {
	////if(ImGui::MenuItem(buff, nullptr, &showPerformanceGraph)) {
	//	//Click to show/hide performance graph
	//	showPerformanceGraph = !showPerformanceGraph;
	//}
	ImGui::Text("%4.0f FPS", ImGui::GetIO().Framerate);

	ImGui::EndMainMenuBar();


	//Draw general settings window
	if (showGeneralSettingsWindow)
	{
		ImGui::Begin("General Settings", &showGeneralSettingsWindow, ImGuiWindowFlags_AlwaysUseWindowPadding);


		if (ImGui::Checkbox("Vertical Sync", &useVerticalSync)) {
			//useVerticalSync = !useVerticalSync;
			ofSetVerticalSync(useVerticalSync);
		}
		//ImGui::SliderInt("Stereo Convergence", &convergence, 0, 100);

		ImGui::End();
	}

	//Draw the log if desired
	if (showLog) {
		Kino::app_log.Draw("Log", &showLog);
	}

	if (showHelp) {
		ImGui::SetNextWindowPosCenter();
		ImGui::OpenPopup("Help");
	}
	if (ImGui::BeginPopupModal("Help", &showHelp, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Columns(2, "HelpColumns", false);
		ImGui::SetColumnOffset(1, 140);

		ImGui::Image(iconTextureID, ImVec2(112, 112));
		ImGui::NextColumn();

		ImGui::TextColored(ImVec4(0.92f, 0.18f, 0.29f, 1.00f), "Kino 1.0.0-alpha");
		ImGui::Text("Jonathan Cole");
		ImGui::Text("Virtual Environment and Multimodal Interaction Laboratory   "); // Window autosizing is broken so add extra spaces here
		if (ImGui::Button("github.com/colejd/kino")) {
			ShellExecute(0, 0, L"https://www.github.com/colejd/kino", 0, 0, SW_SHOW);
		}

		// Draw separator
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 p = ImGui::GetCursorScreenPos();
		draw_list->AddLine(ImVec2(p.x - 9999, p.y), ImVec2(p.x + 9999, p.y), ImGui::GetColorU32(ImGuiCol_Border));

		ImGui::Spacing();
		ImGui::Text("Mouse over any"); ShowHelpMarker("We did it!"); ImGui::SameLine(); ImGui::Text("to show help.");
		ImGui::Text("Ctrl+Click any slider to set its value manually.");

		ImGui::EndPopup();
	}

	//gui.openThemeColorWindow();
}

void ofApp::ToggleTimingWindow()
{
	TIME_SAMPLE_GET_INSTANCE()->setEnabled(!TIME_SAMPLE_GET_INSTANCE()->getEnabled());
}

void ofApp::TakeScreenshot() {
	// Take and save a screenshot
	ofImage img;
	img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
	img.save("Screenshot.png"); // Saved in the data folder
}

// OpenFrameworks Hooks ----------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_ESC) {
		ofExit(EXIT_SUCCESS);
	}
	else if (key == '`') {
		ToggleTimingWindow();
	}
	else if (key == OF_KEY_F1) {
		showHelp = !showHelp;
	}
	else if (key == OF_KEY_F11) {
		// Take and save a screenshot
		TakeScreenshot();
	}
	else if (key == OF_KEY_ALT) {
		modifierPressed = true;
	}
	else if (key == '-' || key == '_') {
		compositor->IncrementConvergence(-1);
	}
	else if (key == '=' || key == '+') {
		compositor->IncrementConvergence(1);
	}
	
	if (modifierPressed) {
		// Functions
		if (key == 'f' || key == 'F') {
			ofToggleFullscreen();
		}
		else if ((key == 's' || key == 'S') && !demoMode) {
			core->swapSides = !(core->swapSides);
		}
		else if (key == 'd' || key == 'D') {
			compositor->doDistortion = !compositor->doDistortion;
		}
		else if (key == 'l' || key == 'L') {
			showLog = !showLog;
		}
		else if (key == 'p' || key == 'P') {
			core->pauseCaptureUpdates = !core->pauseCaptureUpdates;
		}
	}
}

void ofApp::keyReleased(int key) {
	if (key == OF_KEY_ALT) {
		modifierPressed = false;
	}
}

void ofApp::mouseMoved(int x, int y) {

}

void ofApp::mouseDragged(int x, int y, int button) {

}

void ofApp::mousePressed(int x, int y, int button) {

}

void ofApp::mouseReleased(int x, int y, int button) {

}

void ofApp::mouseEntered(int x, int y) {

}

void ofApp::mouseExited(int x, int y) {

}

void ofApp::windowResized(int w, int h) {
	compositor->SetupWindowFBO(w, h);
}

void ofApp::gotMessage(ofMessage msg) {

}

void ofApp::dragEvent(ofDragInfo dragInfo) {

}