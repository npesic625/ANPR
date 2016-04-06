/*
 * TextDetection.h
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#include <core/mat.hpp>

#ifndef ANPR_TEXTDETECTION_H_
#define ANPR_TEXTDETECTION_H_

//Used in SWT algorithm in TextDetection::StrokeWidthTransform
#define imgPixelUpperThreshold 200
#define imgPixelLowerThreshold 50

class TextDetection {
private:
	cv::Mat im_;

	std::vector<cv::Rect> getBoundedTextArea(cv::Mat inputImg);
	cv::Mat combineMatrices (std::vector<cv::Rect> matVec, cv::Mat img, int padding);
	void StrokeWidthTransform(cv::Mat inputImg);
	void ComputeTextFragmentList();

public:
	std::vector<cv::Mat> textSegmentList;

	TextDetection(cv::Mat);
	virtual ~TextDetection();
	std::vector<cv::Mat> getTextMatrixList();
};

struct sortRect
{
	bool operator()(const cv::Rect &rect1, const cv::Rect &rect2)
	{
		return rect1.x < rect2.x;
	}
};

#endif /* ANPR_TEXTDETECTION_H_ */

