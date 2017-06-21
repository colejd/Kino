//
//  ParallelContourDetector.hpp
//  FaceOff
//
//  Created by Jonathan Cole on 10/29/15.
//
//

#ifndef ParallelContourDetector_hpp
#define ParallelContourDetector_hpp

#include "opencv2/opencv.hpp"
#include "ofxTimeMeasurements.h"

/**
 * Takes an image, divides it up, and processes it in separate threads using TBB.
 */
class ParallelContourDetector : public cv::ParallelLoopBody {
private:
	cv::Mat input;
	cv::Mat output;
	int subsections;
	int lineThickness;
	float minContourSize;

public:

	ParallelContourDetector(cv::Mat& in, cv::Mat& out, int _subsections, int _lineThickness, float _minContourSize);

	virtual void operator ()(const cv::Range &range) const;

	static void DetectContoursParallel(cv::Mat in, cv::Mat& out, const int subsections, const int lineThickness, const float _minContourSize);

	static void DetectContours(cv::InputArray in, cv::OutputArray out, const int lineThickness);

	void RepairImage(cv::Mat & mat);


};

#endif /* ParallelContourDetector_hpp */
