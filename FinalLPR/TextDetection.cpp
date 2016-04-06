/*
 * TextDetection.cpp
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#include "TextDetection.h"

#include <core/cvdef.h>
#include <core/mat.inl.hpp>
#include <core/matx.hpp>
#include <core/types.hpp>
#include <core.hpp>
#include <imgproc/imgproc_c.h>
#include <imgproc/types_c.h>
#include <imgproc.hpp>
#include <algorithm>
#include <cassert>
#include <vector>

#include "ImageProcessor.h"
#include "Logger.h"

const char* tesseractLanguage = "eng";
const int filterTextWidthThreshold = 20;
const int filterTextHeightThreshold = filterTextWidthThreshold * 3;


TextDetection::TextDetection(cv::Mat inputImg) {

	assert(inputImg.empty() == 0);

	this->im_ = inputImg;
}

TextDetection::~TextDetection() {
	// TODO Auto-generated destructor stub
}

/*
 * Process the input image to derive text segments or blocks.
 * The detected text fragments will then we assigned if they
 * meet the minimum length/width requirements defined in
 * TextDetection.h.
 *
 * Reference image of TextDetection class must of colour type
 * for debug mode
 */
void TextDetection::ComputeTextFragmentList()
{
	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"TextDetection::ComputeTextFragmentList():",2);
	assert(!im_.empty());

	int avgHeight = 0, imgHeight = im_.rows;
	int yPosAcceptableThreshold = 5;
	int borderWidth = 5;
	int borderCnst = 255;
	ImageProcessor imgProc(im_);
	Logger::lInstance()->log(Logger::blockType::IMAGE, imgProc.getLocalImage(), "Input Img", Logger::logLevel::DEBUG_LEVEL_8);
	imgProc.applyDefaultNormToLocalImage();
	Logger::lInstance()->log(Logger::blockType::IMAGE, imgProc.getLocalImage(), "Normalized Img", Logger::logLevel::DEBUG_LEVEL_8);
	std::vector<cv::Rect> boundedTextRegions = getBoundedTextArea(imgProc.getLocalImage());
	std::vector<cv::Rect> filteredBTextRegions;

	//ImageProcessor imgProc(im_);
	imgProc.applyDefaultNormToLocalImage();
	cv::Mat imGS = imgProc.getGrayScaleImage();
	cv::Mat imEnhGS = imgProc.getEnhancedGSImage();
	Logger::lInstance()->log(Logger::blockType::IMAGE, imGS, "GrayScale Img", Logger::logLevel::DEBUG_LEVEL_8);
	Logger::lInstance()->log(Logger::blockType::IMAGE, imEnhGS, "Enhanced GS Img", Logger::logLevel::DEBUG_LEVEL_8);

	//calculate average height to filter unwanted regions
	for (cv::Rect t: boundedTextRegions)
	{
		avgHeight += t.height;
	}
	//TODO reevaluate this calculations. Potentially flawed if too many or too little bounding regions are detected
	avgHeight = avgHeight / boundedTextRegions.size();

	//filter based on y position
	int yPosAcceptable = (imgHeight - avgHeight)/2;
	for (cv::Rect t: boundedTextRegions)
	{
		if (//(t.y > yPosAcceptable - yPosAcceptableThreshold && t.y < yPosAcceptable + yPosAcceptableThreshold) &&
				(t.height > filterTextHeightThreshold && t.width > filterTextWidthThreshold))
		{
			cv::rectangle(imGS, t, cv::Scalar(0, 255, 0), 2);
			filteredBTextRegions.push_back(t);
		}
	}

	if (filteredBTextRegions.empty() || boundedTextRegions.empty())
	{
		//nothing detected. Terminate method execution
		Logger::lInstance()->log("Nothing detected. Terminate method execution",Logger::logLevel::DEBUG_MAXIMUM);
		return;
	}

	//sort filtered ROIs by position from left to right.
	std::sort(filteredBTextRegions.begin(), filteredBTextRegions.end(), sortRect());
	//Merge filtered + sorted ROIs into single image for processing
	cv::Mat finalMatrix = combineMatrices(filteredBTextRegions,imEnhGS,1);
	//add bordered image to list for processing.
	ImageProcessor iProc(finalMatrix);
	textSegmentList.push_back(iProc.addBorder(iProc.getBinaryImage(),borderWidth,borderCnst));
	Logger::lInstance()->closeLastHeaderBlock();
}

cv::Mat TextDetection::combineMatrices (std::vector<cv::Rect> matVec, cv::Mat img, int padding)
{
	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"TextDetection::combineMatrices:",2);

	int rowSize = 0;
	int colSize = 0;
	int nextColRangeStart = 0;

	for (cv::Rect t: matVec)
	{
		colSize += t.width;
		if (t.height > rowSize)
		{
			rowSize = t.height;
		}
	}
	//add padding to individual bounding objects
	colSize += (matVec.size() + 1) * padding;
	rowSize += padding * 2;


	/* Initialize mat to white image so that combining unequal matrices
	 * will not result in new edges.
	 */
	cv::Mat finalMatrix = cv::Mat::zeros(rowSize, colSize, CV_8UC1);
	finalMatrix = cv::Scalar(255);

	int i = 0;
	for (cv::Rect t: matVec)
	{
		Logger::lInstance()->log(Logger::blockType::IMAGE, cv::Mat(img, t), "Image segment " + std::to_string(i++), Logger::logLevel::DEBUG_MAXIMUM);
		cv::Mat(img, t).copyTo(
				finalMatrix.colRange(nextColRangeStart + padding,
						t.width + nextColRangeStart + padding).rowRange(
						0 + padding, t.height + padding));
		nextColRangeStart += t.width + padding;
	}
	Logger::lInstance()->log(Logger::blockType::IMAGE, finalMatrix, "Image segments - concatenated ", Logger::logLevel::DEBUG_LEVEL_6);
	Logger::lInstance()->closeLastHeaderBlock();
	return finalMatrix;
}

/*
 * Isolate text regions and return list of Rect indicating these
 * regions. Requires input as colour image for debugging.
 *
 * 	Author: dhanushka
 * 	Source: https://stackoverflow.com/questions/23506105/extracting-text-opencv/23672571#23672571
 * 	Source: http://docs.opencv.org/2.4/doc/tutorials/imgproc/shapedescriptors/find_contours/find_contours.html
 */
std::vector<cv::Rect> TextDetection::getBoundedTextArea(cv::Mat inputImg)
{
	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"TextDetection::getBoundedTextArea:",2);
	assert(inputImg.channels() == 3);

	std::vector<cv::Rect> boundedTextRegions;
	cv::Mat gradientImg, binImg, connectedImg, iImg, dilated;
	ImageProcessor imProc(inputImg);

	imProc.applyDefaultNormToLocalImage();
	iImg = imProc.getGrayScaleImage();

	cv::Mat morphKernel = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
	morphologyEx(iImg, gradientImg, cv::MORPH_GRADIENT, morphKernel);
	threshold(gradientImg, binImg, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
	 //adaptiveThreshold(gradientImg, binImg,255,cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY,7,10);
	morphKernel = getStructuringElement(cv::MORPH_RECT, cv::Size(9, 1));
	morphologyEx(binImg, connectedImg, cv::MORPH_CLOSE, morphKernel);

	cv::Mat mask = cv::Mat::zeros(binImg.rows, binImg.cols, CV_8UC1);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	//ignores contours within other counters. May pose problem if entire plate is identified as contour.
	//TODO reevaluate use of CV_RETR_EXTERNAL over CV_RETR_CCOMP when sorting/filter is implemented.
	findContours(connectedImg, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	 for(int idx = 0; idx >= 0; idx = hierarchy[idx][0])
	    {
	        cv::Rect rect = cv::boundingRect(contours[idx]);
	        cv::Mat maskROI(mask, rect);
	        maskROI = cv::Scalar(0, 0, 0);

	        // fill the contour
	        cv::drawContours(mask, contours, idx, cv::Scalar(255, 255, 255), CV_FILLED);

	        // ratio of non-zero pixels in the filled region
	        double r = (double)countNonZero(maskROI)/(rect.width*rect.height);

	        //TODO Reevaluate fixed value. Size may be more acceptable once normalization standard is determined
	        if (r > .15 /* assume at least 15% of the area is filled if it contains text */
	            &&
	            (rect.height > 8 && rect.width > 8) /* constraints on region size */
	            /* these two conditions alone are not very robust. better to use something
	            like the number of significant peaks in a horizontal projection as a third condition */
	            )
	        {
	        	cv::rectangle(iImg, rect, cv::Scalar(0, 255, 0), 2);
	        	boundedTextRegions.push_back(rect);
	        }
	        else
	        {
			std::string rejectionMessage = "rejected: r = " + std::to_string(r)
					+ " height " + std::to_string(rect.height) + " width "
					+ std::to_string(rect.width) + " x pos " + std::to_string(rect.x) + " y pos " + std::to_string(rect.y);
			Logger::lInstance()->log(rejectionMessage,Logger::logLevel::DEBUG_LEVEL_8);
	        }

	    }
	Logger::lInstance()->log(Logger::blockType::IMAGE, iImg, "Contour points", Logger::logLevel::DEBUG_LEVEL_4);
	Logger::lInstance()->closeLastHeaderBlock();
	return boundedTextRegions;
}

/*
 *  Stroke Width Transform according to "Detecting Text in Natural Scenes with Stroke Width Transform"
 *
 *  Epshtein,B , Efek,E, Wexler,Y ,Detecting Text in Natural Scenes with Stroke Width Transform.
 *  Retrieved January, 2016 Available: https://research.microsoft.com/pubs/149305/1509.pdf
 *
 *  TODO finish implementation
 */
void TextDetection::StrokeWidthTransform(cv::Mat inputImg)
{
	cv::Mat gsImg = im_.clone();
	cv::Mat cEdgeImg, gaussImg;
	cv::Mat gradX, gradY;

	//validate single channel grayscale
	assert(im_.channels() == 1);

	//Canny edge detection
	Canny(im_,cEdgeImg,imgPixelLowerThreshold,imgPixelUpperThreshold);

	//get gradients x and y
	GaussianBlur( cEdgeImg, gaussImg, cv::Size(3,3),0,0);
	Sobel(gaussImg, gradX, im_.depth(), 1, 0); //first derivative in x direction
	Sobel(gaussImg, gradY, im_.depth(), 0, 1); //first derivative in y direction
	//blur the gradient results. Perhaps it is not necessary.
	GaussianBlur( gradX, gradX, cv::Size(3,3),0,0);
	GaussianBlur( gradY, gradY, cv::Size(3,3),0,0);
}

std::vector<cv::Mat> TextDetection::getTextMatrixList()
{
	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"LPR::TextDetection::getTextMatrixList():",2);
	if (textSegmentList.empty())
	{
		ComputeTextFragmentList();
	}
	Logger::lInstance()->closeLastHeaderBlock();
	return textSegmentList;
}




