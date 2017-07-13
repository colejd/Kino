#include "TrackedObjectManager.hpp"

TrackedObjectManager::TrackedObjectManager() {

}

TrackedObjectManager::~TrackedObjectManager() {

}

/**
When adding detections, check each one against the set of objects. If a detection
substantially matches an object by ROI and exact matches by label, we ignore it.
*/
void TrackedObjectManager::IngestDetections(vector<detected_object>& detections) {
	TS_SCOPE("Tracking Ingest");
	for (detected_object d : detections) {
		if (!AlreadyTrackingObject(d)) {
			trackers.push_back(TrackerInstance(d));

		}
	}

}

/**
Updates all trackers using `image` as input. If a tracker fails to update,
it is destroyed.
*/
void TrackedObjectManager::UpdateTracking(InputArray image) {
	TS_SCOPE("Tracking Update");
	vector<TrackerInstance>::iterator it = trackers.begin();
	while (it != trackers.end()) {

		cv::UMat umat;
		image.copyTo(umat);

		bool tracking = it->Update(umat);
		if (!tracking) {
			// Remove the tracker from the set
			it = trackers.erase(it);
		}
		else ++it;
	}
}

bool TrackedObjectManager::AlreadyTrackingObject(detected_object detection) {
	cv::Rect rect = cv::Rect(detection.rect.x, detection.rect.y, detection.rect.width, detection.rect.height);

	for (TrackerInstance tracker : trackers) {

		// Check if the tracker has an identical label and similar ROI
		bool sameLabel = detection.label == tracker.detection.label;
		if (!sameLabel) continue; // Skip right away to save time

		// Check if the tracker has a similar ROI to the detection's rect
		bool similarLeft = abs(rect.x - tracker.roi.x) < rectSimilarityThreshold;
		bool similarTop = abs(rect.y - tracker.roi.y) < rectSimilarityThreshold;
		bool similarRight = abs((rect.x + rect.width) - (tracker.roi.x + tracker.roi.width)) < rectSimilarityThreshold;
		bool similarBottom = abs((rect.y + rect.height) - (tracker.roi.y + tracker.roi.height)) < rectSimilarityThreshold;

		bool similarROI = similarLeft && similarRight && similarTop && similarBottom;


		if (sameLabel && similarROI) return true;

	}


	return false;
}

void TrackedObjectManager::DrawBoundingBoxes(InputOutputArray mat) {
	TS_SCOPE("Tracking Draw");
	for (TrackerInstance tracker : trackers) {

		int lineThickness = 4;

		Scalar color = CV_RGB(0, 255, 0);

		cv::Point origin(tracker.roi.x, tracker.roi.y);
		cv::Point opposite(tracker.roi.x + tracker.roi.width, tracker.roi.y + tracker.roi.height);
		// Draw the bounding box
		// Random color: Scalar(rand() & 255, rand() & 255, rand() & 255)
		cv::rectangle(mat, origin, opposite, color, lineThickness, LINE_4);

		// Draw the label in the bounding box
		int fontface = cv::FONT_HERSHEY_DUPLEX;
		double scale = 0.8;
		int textThickness = 1;
		int baseline = 0;
		string label = tracker.detection.label;

		cv::Size text = cv::getTextSize(label, fontface, scale, textThickness, &baseline);

		// Interior label
		cv::Point rectOrigin = cv::Point(tracker.roi.x + (lineThickness / 2.0), tracker.roi.y + text.height + (lineThickness / 2.0));
		cv::rectangle(mat, rectOrigin + cv::Point(0, baseline), rectOrigin + cv::Point(text.width, -text.height), color, CV_FILLED);

		// Exterior label
		//cv::Point origin = cv::Point(rect.x - (lineThickness / 2.0), rect.y - text.height - (lineThickness / 2.0) + baseline); // Exterior label
		//cv::rectangle(drawingMat, origin + cv::Point(0, baseline), origin + cv::Point(text.width, -text.height), color, CV_FILLED);

		cv::putText(mat, label, rectOrigin, fontface, scale, CV_RGB(0, 0, 0), textThickness, LINE_AA);

	}
}

void TrackedObjectManager::DrawGUIPanel(string id) {

	int maxLines = 10; // Number of lines in the scroll box


					   // Draw header
	ImGui::TextColored(ImColor(255, 255, 0), " %-10s ", id.c_str()); // spacing is deliberate
	ImGui::SameLine();
	ImGui::Text(" Tracking %i objects", trackers.size());

	// Draw scroll box
	ImGui::BeginChild(("Scrollbox##" + id).c_str(), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * maxLines), true, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 5));

	ImGuiListClipper clipper(trackers.size(), ImGui::GetTextLineHeightWithSpacing());
	for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {

		ImGui::Text("%s ", trackers[i].detection.label.c_str()); // Left pad the line with maxCharsInLine length

	}
	clipper.End();

	ImGui::PopStyleVar(2);
	ImGui::EndChild();

}
