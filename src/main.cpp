#include "ofMain.h"
#include "ofApp.h"

#include "KinoGlobals.hpp"
#include "config/ConfigHandler.hpp"

//Disable the console
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

//========================================================================
int main() {

	//Disable the console window if requested from the config
	if (ConfigHandler::GetValue("ENABLE_CONSOLE", true).asBool() == false) {
		FreeConsole();
	}

	// Set system flags to prevent the computer from going into sleep mode.
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);

	//Create the app window
	//ofAppGlutWindow window;
	//ofAppGLFWWindow window;
	//ofSetupOpenGL(&window, 640, 480, OF_WINDOW);

	ofGLWindowSettings settings;
	{
		settings.setGLVersion(4, 1);
		settings.width = ConfigHandler::GetValue("WINDOW_START_WIDTH", 640).asInt();
		settings.height = ConfigHandler::GetValue("WINDOW_START_HEIGHT", 480).asInt();
		int winX = ConfigHandler::GetValue("WINDOW_START_X", 0).asInt();
		int winY = ConfigHandler::GetValue("WINDOW_START_Y", 0).asInt();
		settings.setPosition(ofVec2f(winX, winY));
		bool startFullscreen = ConfigHandler::GetValue("WINDOW_START_FULLSCREEN", false).asBool();
		settings.windowMode = startFullscreen ? OF_FULLSCREEN : OF_WINDOW;
		settings.title = "Kino";
	}
	ofCreateWindow(settings);

	ofSetVerticalSync(false);

	//Start running the app
	ofRunApp(new ofApp()); //Loops here until the app is told to exit.
	//If you reach this point, the app is in the process of exiting.

	std::cout << "Kino has finished running." << std::endl;
	return 0;
}
