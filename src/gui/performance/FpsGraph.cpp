#include "FpsGraph.hpp"

FpsGraph::FpsGraph() {
	fpsqueue = deque<float>(queueSize);
	fpsqueue.assign(0.0f, queueSize);
}

FpsGraph::~FpsGraph()
{
}

/**
* Push an FPS value to the front of the queue to be drawn.
*/
void FpsGraph::Enqueue(float fps) {
	if (doAveraging) {
		if (currentIndex < itemsInAveragingGroup) {
			currentTotal += fps;
			currentIndex += 1;
		}
		else {
			fpsqueue.push_front(currentTotal / itemsInAveragingGroup);
			currentTotal = 0;
			currentIndex = 0;
		}
	}
	else {
		fpsqueue.push_front(fps);
	}
	//if (fps > yMax) yMax = fps;
	fpsqueue.resize(queueSize);
}

/**
* Draws a graph representing fpsqueue to the screen at (xpos, ypos) (the bottom left corner).
* Lots of magic numbers, but it looks good for now.
*/
void FpsGraph::Draw(float x, float y, float w, float h, std::string label) {
	float baseY = y + h; //Bottom of the graph
	float displayedFPS = 0.0f;
	if (fpsqueue.size() > 0) displayedFPS = fpsqueue.at(0);

	//Draw label
	if (label.empty()) {
		label = "FPS";
	}
	ofSetColor(255, 255, 255);
	ofDrawBitmapString(label, x - 1, y - 7);

	//Draw background box
	ofSetColor(0, 0, 0);
	ofRect(x, y, w, h);

	//Draw graph
	ofSetHexColor(0x43A047);
	ofPolyline graphLine;
	for (int i = queueSize - 1; i>= 0; i--) {
		float fps = fpsqueue.at(i);
		float innerX = inverseLerp(0, queueSize - 1, i) * w;
		float innerY = inverseLerp(lowerFpsLine, upperFpsLine, fps) * h;
		//if (fillGraph) graphLine.addVertex(x + innerX, y + 0);
		graphLine.addVertex(x + w - innerX, baseY - innerY);
	}
	if (fillGraph) {
		//Add closing vertices along bottom of graph
		graphLine.addVertex(x + w, baseY);
		graphLine.addVertex(x, baseY); //Close
		//graphLine.close();
		//graphLine.draw();
		//OpenGL method (faster)
		ofBeginShape();
		for (int i = 0; i < graphLine.getVertices().size(); i++) {
			ofVertex(graphLine.getVertices().at(i).x, graphLine.getVertices().at(i).y);
		}
		ofEndShape();
	}
	else {
		graphLine.draw();
	}

	//Draw target line
	ofSetColor(255, 0, 0);
	ofPolyline targetLine;
	float targetLineInnerY = inverseLerp(lowerFpsLine, upperFpsLine, targetFps) * h;
	targetLine.addVertex(x, baseY - targetLineInnerY);
	targetLine.addVertex(x + w, baseY - targetLineInnerY);
	targetLine.draw();
	//Draw target line FPS
	ofDrawBitmapString(ofToString(targetFps, 0), x + 3, baseY - targetLineInnerY + 12);

	//Draw FPS string
	ofSetColor(255, 255, 255);
	if (displayedFPS < targetFps) ofSetColor(255, 0, 0);
	ofDrawBitmapString(ofToString(displayedFPS, 0), x + 3, y + 10);

	//Draw border box
	ofSetColor(255);
	ofPolyline rect;
	rect.addVertex(x, y);
	rect.addVertex(x + w, y);
	rect.addVertex(x + w, y + h);
	rect.addVertex(x, y + h);
	rect.addVertex(x, y); //Close
	rect.draw();

	ofSetColor(255);

}

/**
* Gives the progress of val between lower and upper as a float between 0 and 1.
*/
float inverseLerp(const float lower, const float upper, const float val) {
	//newValue = (convertedValue - minHeight) / (maxHeight - minHeight);
	float ret = (val - lower) / (upper - lower);
	//if (ret < 0.0f) ret = 0.0f;
	//if (ret > 1.0f) ret = 1.0f;
	//return ret;
	return std::max(0.0f, std::min(ret, 1.0f)); //Clamp between 0 and 1
}