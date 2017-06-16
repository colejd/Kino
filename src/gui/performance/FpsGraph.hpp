#pragma once

#include <deque>

#include "ofMain.h"
#include <opencv2/flann/timer.h>

#define START_TIMER(name) cvflann::StartStopTimer name = cvflann::StartStopTimer(); name.start();

#define STOP_TIMER(name) name.stop();

#define PRINT_TIMER(name, label) printf("%s: %.1fms\n", label, name.value * 1000);

#define GET_TIMER_MS(name) (float)(name.value * 1000);

class FpsGraph
{
public:
	FpsGraph();
	~FpsGraph();

	deque <float> fpsqueue;

	int queueSize = 180;

	float targetFps = 60.0f;
	float upperFpsLine = 300.0f;
	float lowerFpsLine = 0.0f;

	void Enqueue(const float fps);
	void Draw(float x, float y, float w, float h, std::string label = "");
	bool fillGraph = false;

	bool doAveraging = false;
	int itemsInAveragingGroup = 10;
	int currentIndex = 0;
	float currentTotal = 0.0f;
};

/**
* Gives the progress of val between lower and upper as a float between 0 and 1.
*/
float inverseLerp(const float lower, const float upper, const float val);
