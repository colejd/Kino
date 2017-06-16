#pragma once

#include "ofMain.h"

#include "ofxImGui.h"

#include "KinoCore.hpp"
#include "compositor/ImageCompositor.hpp"
#include "config/ConfigHandler.hpp"
#include "gui/UsesGUI.hpp"
#include "gui/performance/FpsGraph.hpp"

#include <opencv2/opencv.hpp>

class ofApp : public ofBaseApp, public UsesGUI {
	public:
		ofApp();
		~ofApp();

		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		void DrawGUI();
		void DrawAllGUI();
		
		KinoCore *core;
		ImageCompositor *compositor;

		bool showGeneralSettingsWindow = false;
		bool showLog = false;					//Show log window
		bool showHelp = false;					//Show help window

		void ToggleTimingWindow();

		void SetGUITheme();

		FpsGraph fpsGraph = FpsGraph();
		bool showPerformanceGraph = false;		//FpsGraph visibility on screen

		float coreFps;
		float msFrameTarget = 1000.0f / 60.0f;

		bool useVerticalSync = false;

		ofxImGui::Gui gui;

};
