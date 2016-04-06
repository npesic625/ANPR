/*
 * LPR.cpp
 *
 *  Created on: 2016-03-01
 *      Author: npesic
 */

#include <baseapi.h>
#include <cv.hpp>
#include <iostream>

#include "IMG.h"
#include "Logger.h"
#include "TextDetection.h"


using namespace cv;
using namespace std;

extern const char* tesseractLanguage;
std::vector<std::string> lpList;
std::vector<cv::Mat> textSegmentMatList;
// const string defaultJPG = "/home/npesic/Documents/car.JPG";
const string defaultJPG = "3.jpg";


int main( int argc, char** argv )
{

	string openJPG = "ReferenceImages/RawCarImages/";

    if( argc > 2 )
    {
     cout <<" Usage: LPR ImageToLoadAndDisplay" << endl;
     return -1;
    }
    else if( argc == 1 )
    	openJPG += defaultJPG;
    else
    	openJPG += argv[1];

    cout << openJPG << endl;
	//startup Logger
	if (!Logger::lInstance()->isInitialized())
	{
		Logger::lInstance()->initialize("LicensePlateRecognition");
		Logger::lInstance()->setLogLevel(Logger::logLevel::DEBUG_MAXIMUM);

		Logger::lInstance()->logHeader(Logger::blockType::HEADER_H1,"Event Log",2);
		Logger::lInstance()->logHeader(Logger::blockType::CATEGORY_HEADER, "Results" ,10);
	}
    IMG image(openJPG);

	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H2,"Original Image", 2);
	Logger::lInstance()->log(Logger::blockType::IMAGE, image.getOrig(), openJPG, Logger::logLevel::DEBUG_MINIMUM);

	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H2,"Masked Plate Image", 4);
	Logger::lInstance()->log(Logger::blockType::IMAGE, image.getMaskedPlate(), "Masked Plate Image", Logger::logLevel::DEBUG_MINIMUM);

	image.GrayScale(true);

    image.Binary(true);

    image.Edge(true);

    image.Contour(true);

	Logger::lInstance()->logHeader(Logger::blockType::HEADER_H3,"Plate section", 10);
	Logger::lInstance()->log(Logger::blockType::IMAGE, image.getPlate(), "Plate section", Logger::logLevel::DEBUG_MINIMUM);

//    im.ParallelLines(false);

	//initialize Tesseract
	tesseract::TessBaseAPI tess;
	tess.Init(NULL, tesseractLanguage, tesseract::OEM_DEFAULT);
	tess.SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);
	tess.SetVariable("tessedit_char_whitelist", "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345789");

	//initialize and get text segements in cv::Mat format
	TextDetection localTD(image.getBigPlate());
	textSegmentMatList = localTD.getTextMatrixList();

	if (!textSegmentMatList.empty())
	{
		for(cv::Mat lpSegment :  textSegmentMatList)
		{
			Logger::lInstance()->log(Logger::blockType::IMAGE, lpSegment, "Image Text Segement", Logger::logLevel::DEBUG_LEVEL_6);
			tess.SetImage((uchar*) lpSegment.data, lpSegment.cols, lpSegment.rows,1, lpSegment.cols);
			lpList.push_back(tess.GetUTF8Text());
			Logger::lInstance()->log("Result: " + lpList.back(),Logger::logLevel::DEBUG_MAXIMUM);
		}
	}
	else
	{
		lpList.push_back("No valid characters detected");
		Logger::lInstance()->log("Result: No valid characters detected",Logger::logLevel::DEBUG_MAXIMUM);
	}

	waitKey(0);

	Logger::lInstance()->closeLastHeaderBlock();
	Logger::lInstance()->closeFile();
	cout << endl << "LicensePlateRecognition will now terminate. " << endl;

}
