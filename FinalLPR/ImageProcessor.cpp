/*
 * ImageProcessor.cpp
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#include "ImageProcessor.h"

#include <core/base.hpp>
#include <core/hal/interface.h>
#include <core/mat.hpp>
#include <core/mat.inl.hpp>
#include <core/operations.hpp>
#include <core/types.hpp>
#include <core.hpp>
#include <highgui.hpp>
#include <imgcodecs/imgcodecs_c.h>
#include <imgcodecs.hpp>
#include <imgproc/types_c.h>
#include <imgproc.hpp>
#include <magick/colorspace.h>
#include <magick/constitute.h>
#include <Magick++/Image.h>
#include <sys/stat.h>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

#include "Logger.h"


ImageProcessor::ImageProcessor(std::string fName) {
	//initialize Magick++
	Magick::InitializeMagick("");

	fileName = fName;

	if (!fileExists(fName)) {
		throw std::invalid_argument(
				"File does not exist. Correct shortcoming and rerun");
	}

	//process colour + greyscale + binary images
	im_ = cv::imread(fileName, CV_LOAD_IMAGE_COLOR);
}

ImageProcessor::ImageProcessor(cv::Mat imAsMat) {
	//initialize Magick++
	Magick::InitializeMagick("");
	assert(!imAsMat.empty());

	//process colour + greyscale + binary images
	if (imAsMat.channels() != 3)
	{
		//not possible to actually convert to full colour
		//simply converts to 3 channel image
		cv::cvtColor(imAsMat, im_, CV_GRAY2BGR);
	}
	else
	{
		this->im_ = imAsMat;
	}
}

ImageProcessor::~ImageProcessor() {
	// TODO Auto-generated destructor stub
}

/*
 * Author: datahaki
 * Source: https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
 */
bool ImageProcessor::fileExists(const std::string fName) {
	struct stat buf;
	return (stat(fName.c_str(), &buf) == 0);
}

cv::Mat ImageProcessor::getLocalImage()
{
	return this->im_;
}

void ImageProcessor::toGrayScale() {
	if (im_.channels() == 3)
	{
		//if colour image
		cvtColor(im_, im_gray, cv::COLOR_BGR2GRAY);
	}
	else
	{
		//image is already a grayscale image
		im_gray = im_;
	}

	if (im_gray.empty()) {
		std::cout << "toGrayScale() error reading image. im_gray is empty" << std::endl;
		std::cout << "channels = " << im_.channels() << std::endl;
		std::cout << "im_ is empty? = " << im_.empty() << std::endl;
		throw std::invalid_argument("Error with toGrayscale method");
	}
}
/*
 *  Displays grayscale image
 */
void ImageProcessor::displayGrayScale() {

	cv::imshow("im_gray", this->getGrayScaleImage());
	cv::waitKey(0);
}


cv::Mat ImageProcessor::getGrayScaleImage()
{
	if (im_gray.empty())
	{
		toGrayScale();
	}

	return im_gray;
}
/*
 * Binarizes image using GaussianBluring and OTSU thresholding.
 *
 * Source:http://docs.opencv.org/master/d7/d4d/tutorial_py_thresholding.html#gsc.tab=0
 */
//TODO revaluate using enhanced grayscale image as baseline image rather than grayscale image
void ImageProcessor::binarizeGSImage() {
	double minValue = 0;
	double maxValue = 255;
	cv::Mat blur;

	cv::GaussianBlur(this->getGrayScaleImage(),blur,cv::Size(5,5),0,0);
	cv::threshold(blur, im_binary, minValue, maxValue, CV_THRESH_BINARY | cv::THRESH_OTSU);
}

void ImageProcessor::displayBinaryImage() {

	cv::imshow("im_binary", this->getBinaryImage());
	cv::waitKey(0);
}

cv::Mat ImageProcessor::getBinaryImage()
{
	if (im_binary.empty())
	{
		binarizeGSImage();
	}

	return im_binary;
}

cv::Mat ImageProcessor::addBorder(cv::Mat inputImg, int borderWidth, int value)
{
	cv::Mat borderedImg;
	cv::copyMakeBorder(inputImg, borderedImg , borderWidth, borderWidth, borderWidth, borderWidth, cv::BORDER_CONSTANT, value);

	return borderedImg;
}
/*
 * This process weakens background information while enhancing foreground information.
 * The result should be an image with enhanced lettering. The reference image should
 * already be cropped.
 *
 * https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
 */
void ImageProcessor::enhanceGrayScaleImage() {

	im_gray_enhanced = this->getGrayScaleImage().clone();
	std::vector<int> matArray;
	int pixelEnhancementThreshold;
	int nrows = im_gray_enhanced.rows;
	int ncols = im_gray_enhanced.cols;
	uchar* ptr;

	//quantize to range of 1 to 100 from 255
	im_gray_enhanced *= (1 / 2.55);

	//initialize vector of standard grayscale image
	if (im_gray_enhanced.isContinuous()) {
		matArray.assign(im_gray_enhanced.datastart, im_gray_enhanced.dataend);
	} else {
		for (int i = 0; i < im_gray_enhanced.rows; i++) {
			matArray.insert(matArray.end(), im_gray_enhanced.ptr<int>(i),
					im_gray_enhanced.ptr<int>(i) + im_gray_enhanced.cols);
		}
	}

	//sort vector from smallest to greatest
	std::sort(matArray.begin(), matArray.end());

	if (im_gray_enhanced.isContinuous()) {
		ncols *= nrows;
		nrows = 1;
	}

	cv::Mat localBinaryImg = this->getBinaryImage(); //use this to ensure the binary image is present
	double r = (double)countNonZero(localBinaryImg) / localBinaryImg.total();
	pixelEnhancementThreshold = matArray.at(matArray.size() * (1-r));
	for (int i = 0; i < nrows; i++) {
		ptr = im_gray_enhanced.ptr<uchar>(i);
		for (int j = 0; j < ncols; j++) {
			if (ptr[j] > pixelEnhancementThreshold) {
				ptr[j] *= 2.55;
			}
		}
	}
}

void ImageProcessor::displayEnhancedGS() {
	if (im_gray_enhanced.empty()) {
		enhanceGrayScaleImage();
	}

	cv::imshow("im_gray_enhanced", im_gray_enhanced);
	cv::waitKey(0);
}

cv::Mat ImageProcessor::getEnhancedGSImage()
{
	if (im_gray_enhanced.empty())
	{
		enhanceGrayScaleImage();
	}
	return im_gray_enhanced;
}



/*
 *  This method will normalize a pre-processed license plate image to a specified
 *  height while retaining resolution. An ideal image will be of font size 14 at 50 -100
 *  pixels in height.
 *
 *  @inputImg: input image in terms of cv::Mat
 *  @height: height or rows for new image.
 *  @width: width or columns for new image. Closest value will be used if normalizeMode = RETAIN_ASPECT_RATIO
 *  @normalizeMode: Mode to use for normalization of image size.
 */
cv::Mat ImageProcessor::normalizeLPImage(cv::Mat inputImg, int width, int height, int normalizeMode)
{
	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"ImageProcessor::normalizeLPImage():",2);
	Logger::lInstance()->log("width: " + std::to_string(width) + "height: " + std::to_string(height) + "normalizeMode: " + std::to_string(normalizeMode) ,Logger::DEBUG_LEVEL_6);
	assert(!inputImg.empty());
	assert(height > 0);
	assert(width > 0);

	//resize image
	Magick::Image testIm;
	Magick::Image resizedIm;
	cv::Mat img = inputImg;
	double aspectRatio = 0;
	int finalHeight = 0;
	int finalWidth = 0;

	switch(normalizeMode)
	{
	case RETAIN_ASPECT_RATIO:
		aspectRatio = inputImg.cols / inputImg.rows;
		finalHeight = (double) height * aspectRatio;
		finalWidth  = (double) width * aspectRatio;
		break;
	case IGNORE_ASPECT_RATIO:
		finalHeight = height;
		finalWidth = width;
		break;
	}

	/*	cv::resize(img,img,cv::Size(finalWidth, finalHeight), 0, 0, cv::INTER_CUBIC);*/

	testIm = convMatToMagickImage(img);

	//despeckle
	testIm.despeckle();

	//reduce noise/enhance
	testIm.reduceNoise(0.5);

	//sharpen image
	testIm.unsharpmask(2,1,0.7,0.02);
	testIm.enhance();

	Logger::lInstance()->closeLastHeaderBlock();
	return convMagickImageToMat(testIm);
}

void ImageProcessor::applyDefaultNormToLocalImage()
{
	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"ImageProcessor::applyDefaultNormToLocalImage()",2);
	Logger::lInstance()->log("width: " + std::to_string(500) + "height: " + std::to_string(250) + "normalizeMode: RETAIN_ASPECT_RATIO" ,Logger::DEBUG_LEVEL_6);
	this->im_ = normalizeLPImage(this->im_, 500, 250, ImageProcessor::RETAIN_ASPECT_RATIO);

	//redefine GS and Binary images if already defined
	if (!this->im_gray.empty())
	{
		toGrayScale();
	}

	if (!this->im_gray.empty())
	{
		binarizeGSImage();
	}
	Logger::lInstance()->closeLastHeaderBlock();
}

/*
 * Convert cv::Mat to Magick::Image
 */
Magick::Image ImageProcessor::convMatToMagickImage(cv::Mat &matImg)
{
	assert(!matImg.empty());

	if (matImg.channels() == 1)
	{
		Magick::Image img(matImg.cols, matImg.rows, "R", Magick::CharPixel, (uchar *)matImg.data);
		return img;
	}
	else
	{
		Magick::Image img(matImg.cols, matImg.rows, "BGR", Magick::CharPixel, (uchar *)matImg.data);
		return img;
	}
}

/*
 * Convert Magick::Image to cv::Mat
 */
cv::Mat ImageProcessor::convMagickImageToMat(Magick::Image magickImg)
{
	assert(magickImg.isValid());

	cv::Mat convertedMat;
	//TODO find better way to transfer data from image to mat
	magickImg.write("aa.jpg");

	if (magickImg.colorSpace() == Magick::GRAYColorspace)
	{
		/*
		 * hack to transfer data. Early tests show that direct mat -> image -> mat results
		 * in scrambled image. Rather
		 */
		convertedMat = cv::imread("aa.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	}
	else
	{
		convertedMat = cv::imread("aa.jpg", CV_LOAD_IMAGE_COLOR);
	}

	return convertedMat;
}



