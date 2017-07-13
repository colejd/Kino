#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

#include "KinoGlobals.hpp"
#include "ofxTimeMeasurements.h"

#include "ofMain.h"
#include "ofxDarknet.h"
#include "ofxImGui.h"

using namespace std;
using namespace cv;

class TrackerInstance;

class TrackedObjectManager {
public:
	TrackedObjectManager();
	~TrackedObjectManager();

	// Takes a raw set of detections and decides what is new and what
	// is already tracked. Creates new trackers for new objects.
	void IngestDetections(vector<detected_object> &detections);

	// Updates tracking for all objects based on the input image.
	void UpdateTracking(InputArray image);

	void DrawBoundingBoxes(InputOutputArray mat);
	void DrawGUIPanel(string id);


	/**
	Scratch:

	How can I uniquely identify trackers? The most useful info is the rect
	and label from the detection used to create it.


	*/

	vector<TrackerInstance> trackers;

	int rectSimilarityThreshold = 20000; // Pixels


private:
	bool AlreadyTrackingObject(detected_object detection);

};

class TrackerInstance {
public:
	Rect2d roi;
	// These are smart pointers, no need to manually allocate.
	Ptr<Tracker> tracker;

	detected_object detection;

	bool initialized = false;

	TrackerInstance(detected_object detection) {
		this->detection = detection;

		roi = cv::Rect(detection.rect.x, detection.rect.y, detection.rect.width, detection.rect.height);
		tracker = TrackerKCF::create(); // Try MIL too
	}

	bool Update(InputArray frame) {
		if (!initialized) {
			initialized = tracker->init(frame, roi);
		}

		return tracker->update(frame, roi);
	}
};

