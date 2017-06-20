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

	demoMode = ConfigHandler::GetValue("WEBCAM_DEMO_MODE", false).asBool();
}

void ofApp::update(){
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
		//Draw the GUI for this class
		DrawGUI();
		//Draw the core GUI and its modules' GUIs
		core->DrawAllGUIs();
		//Draw the compositor's GUI
		compositor->DrawGUI();
	gui.end();
	TS_STOP_NIF("ImGui Draw");
}

void ofApp::DrawGUI() {

	//ImGui::ShowTestWindow();

	// Draw the menu bar
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("Fullscreen", "F")) {
			ofToggleFullscreen();
		}
		ImGui::MenuItem("Help", nullptr, &showHelp);
		if (ImGui::MenuItem("Quit", "ESC")) {
			ofExit(EXIT_SUCCESS);
		}
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Modules")) {
		ImGui::MenuItem("Edge Detection", "E", &(core->edgeDetector.showGUI));
		ImGui::MenuItem("Face Detection", nullptr, &(core->faceDetector.showGUI));
		ImGui::MenuItem("Image Classifier", "C", &(core->classifierLens.showGUI));

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("HMD")) {
		ImGui::MenuItem("Swap Eyes", "S", &(compositor->swapStereoSides));
		ImGui::MenuItem("Distortion", "D", &(compositor->doDistortion));

		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("View")) {
		ImGui::MenuItem("General Settings", nullptr, &showGeneralSettingsWindow);
		ImGui::MenuItem("Compositor Settings", nullptr, &(compositor->showGUI));
		ImGui::MenuItem("Log", "L", &showLog);
		ImGui::MenuItem("FPS Graph", nullptr, &showPerformanceGraph);
		if (ImGui::MenuItem("Performance", nullptr)) {
			ToggleTimingWindow();
		}
		ImGui::EndMenu();
	}

	//Draw core timing
	//ImVec4 textColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	//if (coreFps > frameTarget) textColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); //Set red if over max frame time
	//else if (coreFps > frameTarget / 2.0f) textColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); //Set yellow if halfway to max frame time
	//ImGui::SameLine(ImGui::GetWindowWidth() - 160); ImGui::TextColored(textColor, "core: %.1f ms", coreFps);

	//Draw application fps
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
		ImGui::Checkbox("Pause", &core->pauseCaptureUpdates);
		//ImGui::SliderInt("Stereo Convergence", &convergence, 0, 100);

		ImGui::End();
	}

	//Draw the log if desired
	if (showLog) {
		Kino::app_log.Draw("Log", &showLog);
	}

	if (showHelp) ImGui::OpenPopup("Help");
	//ui::ScopedWindow window( "Help", ImGuiWindowFlags_AlwaysAutoResize );
	if (ImGui::BeginPopupModal("Help", &showHelp)) {
		ImGui::TextColored(ImVec4(0.92f, 0.18f, 0.29f, 1.00f), "Kino 0.1.0");
		ImGui::Text("Jonathan Cole");
		ImGui::Text("Virtual Environment and Multimodal Interaction Laboratory");
		//ImGui::Text("github.com/seieibob/kino");
		//ImGui::SameLine();
		if (ImGui::Button("github.com/seieibob/kino")) {
			ShellExecute(0, 0, L"https://www.github.com/seieibob/kino", 0, 0, SW_SHOW);
		}
		ImGui::Separator();
		//ImGui::Spacing();
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

void ofApp::SetGUITheme() {
	//Style based on this gist:
	//https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9

	ofxImGui::BaseTheme theme = ofxImGui::BaseTheme();//BaseTheme();

	ImGuiStyle& style = ImGui::GetStyle();
	float alpha_ = 0.9f;
	// light style from Pacôme Danhiez (user itamago) 
	// https://github.com/ocornut/imgui/pull/511#issuecomment-175719267
	style.Alpha = 1.0f;
	style.FrameRounding = 3.0f;
	style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.70f, 0.70f, 0.70f, 0.94f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(0.86f, 0.86f, 0.86f, 0.99f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(0.59f, 0.59f, 0.59f, 0.50f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.98f, 0.39f, 0.36f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	//style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);

	for (int i = 0; i <= ImGuiCol_COUNT; i++)
	{
		ImVec4& col = style.Colors[i];
		float H, S, V;
		ImGui::ColorConvertRGBtoHSV(col.x, col.y, col.z, H, S, V);

		if (S < 0.1f)
		{
			V = 1.0f - V;
		}
		ImGui::ColorConvertHSVtoRGB(H, S, V, col.x, col.y, col.z);
		if (col.w < 1.00f)
		{
			col.w *= alpha_;
		}
	}

}

//OpenFrameworks Methods----------------------------------------
void ofApp::keyPressed(int key) {
	if (key == OF_KEY_ESC) {
		ofExit(EXIT_SUCCESS);
	}
	else if (key == '`') {
		ToggleTimingWindow();
	}
	else if (key == 'f' || key == 'F') {
		ofToggleFullscreen();
	}
	else if (key == 's' || key == 'S') {
		compositor->swapStereoSides = !(compositor->swapStereoSides);
	}
	else if (key == 'd' || key == 'D') {
		compositor->doDistortion = !compositor->doDistortion;
	}
	else if (key == '-' || key == '_') {
		compositor->IncrementConvergence(-1);
	}
	else if (key == '=' || key == '+') {
		compositor->IncrementConvergence(1);
	}
	else if (key == 'e' || key == 'E') {
		core->edgeDetector.ToggleGUIEnabled();
	}
	else if (key == 'c' || key == 'C') {
		core->classifierLens.ToggleGUIEnabled();
	}
	else if (key == 'l' || key == 'L') {
		showLog = !showLog;
	}
	else if (key == 'p' || key == 'P') {
		// Take and save a screenshot
		ofImage img;
		img.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
		img.save("Screenshot.png"); // Saved in the data folder
	}
}

void ofApp::keyReleased(int key) {

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