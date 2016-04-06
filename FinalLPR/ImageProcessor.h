/*
 * ImageProcessor.h
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#ifndef IMAGEPROCESSOR_H_
#define IMAGEPROCESSOR_H_

#include <iostream>
#include <cv.h>
#include <Magick++.h>

using namespace Magick;

class ImageProcessor {
private:
	std::string fileName;
	std::string lpNumber;
	cv::Mat im_;
	cv::Mat im_gray;
	cv::Mat im_gray_enhanced;
	cv::Mat im_binary;

	Image convMatToMagickImage(cv::Mat &matImg);
	cv::Mat convMagickImageToMat(Image magickImg);

protected:
	void toGrayScale();
	void binarizeGSImage();
	void enhanceGrayScaleImage();

public:
	ImageProcessor(std::string fName);
	ImageProcessor(cv::Mat imAsMat);
	virtual ~ImageProcessor();

	static bool fileExists(const std::string fName);
	void displayGrayScale();
	void displayBinaryImage();
	void displayEnhancedGS();
	cv::Mat getLocalImage();
	cv::Mat getEnhancedGSImage();
	cv::Mat getGrayScaleImage();
	cv::Mat getBinaryImage();
	cv::Mat addBorder(cv::Mat inputImg, int width, int value);
	cv::Mat normalizeLPImage(cv::Mat inputImg, int width, int height, int normalizeMode);
	void applyDefaultNormToLocalImage();

	enum normalizeMode: int
	{
		RETAIN_ASPECT_RATIO,
		IGNORE_ASPECT_RATIO
	};
};

#endif /* IMAGEPROCESSOR_H_ */

