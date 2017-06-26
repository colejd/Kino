#pragma once

#include "ofMain.h"

#include "gui/UsesGUI.hpp"

#include "ofxTimeMeasurements.h"

#include <opencv2/opencv.hpp>

#include "DistortionManager.hpp"

//#include "ofxOpenVR.h"

/*
Responsible for drawing images from the core to the screen in a nice way.
*/
class ImageCompositor : public UsesGUI {
public:
	ImageCompositor(const int width, const int height);
	~ImageCompositor();

	void SetupWindowFBO(int width, int height);

	void DrawGUI() override;

	/**
	Drawing modes
	*/
	//enum class COMPOSITE_MODE{
	//	/** Obeys stereo rules */
	//	STEREO,
	//	/** 3d composite for super fancy interpolation */
	//	THREEDEE,
	//	/** Draws straight to screen */
	//	NOCOMPOSITE
	//};

	/* Draws a cv::Mat directly to the screen via OpenFrameworks. */
	void DrawMatDirect(const cv::Mat& mat, ofImageType type, int x, int y, string caption);
	/* Draws a cv::Mat directly to the screen via OpenFrameworks. */
	void DrawMatDirect(const cv::Mat& mat, ofImageType type, int x, int y, int w, int h, string caption);

	ofRectangle AspectFillRectToTarget(ofRectangle original, ofRectangle target);

	ofRectangle AspectFitRectToTarget(ofRectangle original, ofRectangle target);

	void DrawMatToFbo(const cv::InputArray input, ofFbo fbo, int convergence = 0);
	void DrawMatFullscreen(const cv::InputArray arr);
	//void DrawMatToFbo(const cv::Mat& mat, ofFbo& fbo);
	void DrawMatsToFbo(const cv::InputArray leftMat, const cv::InputArray rightMat);
	void DrawWindowFbo();

	int convergence = 0;

	//FBO which is scaled to the entire window
	ofFbo windowFbo;

	ofFbo leftFbo;
	ofFbo rightFbo;

	int fboNumSamples = 0;

	ofTrueTypeFont messageFont;

	bool mirror = false;

	int convergenceMin = 0;
	int convergenceMax = 80;
	void IncrementConvergence(int inc);

	//void InitOpenVR();
	//void render(vr::Hmd_Eye nEye);
	void Update();

	struct Size {
		int x;
		int y;
	};
	Size lastSize{ 0, 0 };
	ofImage output;

	DistortionManager distortion;
	bool doDistortion = false;

private:
	//ofxOpenVR _openVR;

	ofImage _texture;
	ofBoxPrimitive _box;
	ofMatrix4x4 _translateMatrix;
	ofShader _shader;

	ofBoxPrimitive _controllerBox;
	ofShader _controllersShader;

	ofLight pointLight;

};