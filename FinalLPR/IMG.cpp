/*
 * IMG.cpp
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#include <opencv/cv.hpp>
#include <opencv/highgui.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/stat.h>

#include <string>

#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace cv;

#include "IMG.h"

int const max_value = 255;
int const bthreshold = 100;
int const cthreshold = 55;

//constructor of object IM
IMG::IMG(string fName) {
// parameter saved to variable fileName
	fileName = fName;

//checks if fileName exists else throws error
	if (!fileExists(fileName)) {
		throw invalid_argument(
				"File does not exist. Correct the file name and rerun");
	}

//outputs filename to Console
	cout <<  fileName << endl ;

//imports file image inputed to Mat variable im
	im = imread( fileName, CV_LOAD_IMAGE_COLOR );

//hsv image used for colour comparison
	cvtColor(im,im_hsv, COLOR_BGR2HSV);

	//resize image if too big or too small
	if ((im.rows < 640 || im.rows > 1280) && (im.cols < 480 || im.cols  > 1280))
		resize(im, im, Size(640,480), 0, 0, INTER_NEAREST);
	cout <<  "Original image: " << im.size() << " Channels: " << im.channels() << endl ;

//titles and displays image in window
	namedWindow( "Display Image", CV_WINDOW_AUTOSIZE );
	imshow( "Display Image", im );

}

//Get Original Image
Mat IMG::getOrig(void){

	return im;
}

//Get Masked Plate Image
Mat IMG::getMaskedPlate(void){

	return im_plate;
}

//Get Plate Image
Mat IMG::getPlate(void){

	return im_justPlate;
}

//Get Resized Plate Image
Mat IMG::getBigPlate(void){

	return im_plateBig;
}

//First step) initalizes im_gray and places gray image to matrix
void IMG::GrayScale(bool show) {
	im_gray.create(im.size(), CV_8U);

//Uses private function to give im_gray value
	toGrayScale(im, im_gray);
	cout <<  "Gray: " << im_gray.size() << " Channels: " << im_gray.channels() << endl ;

//Checks whether gray image exists and displays image else displays
	if(!im_gray.empty()) {

//checks parameter to see whether to display grayscale image in new window
		if(show){
			namedWindow( "Display Gray Image", CV_WINDOW_AUTOSIZE );
			imshow( "Display Gray Image", im_gray );
		}
	} else
	    cout<<"Gray Empty "<<endl;;

}
//Second step) initializes im_binary and places binary image to matrix
void IMG::Binary(bool show) {
	im_binary.create(im.size(), CV_8U);

//if im_gray doesn't exist create gray image to use for later
	if(im_gray.empty()){
		GrayScale(false);
	}
//uses private function to give im_binary value
	toBinary(im_gray, im_binary, bthreshold);
	cout <<  "Binary: " << im_binary.size() << " Channels: " << im_binary.channels() << endl ;

//checks parameter to see whether to display binary image in new window
	if(!im_binary.empty()) {
		if(show){
			namedWindow( "Display Binary Image", CV_WINDOW_AUTOSIZE );
			imshow( "Display Binary Image", im_binary );
		}
	} else
	    cout<<"Binary Empty ";

}
//Third step) initialize im_edge and places edge detected image to matrix
void IMG::Edge(bool show) {
	im_edge.create(im.size(), CV_8U);

//if im_binary doesn't exist create binary image to use for later
	if(im_binary.empty()){
		Binary(false);
	}
//uses private function to give im_edge value
	edgeDetection(im_binary, im_edge, cthreshold);
	cout <<  "Edge: " << im_edge.size() << " Channels: " << im_edge.channels() << endl ;

//checks parameter to see whether to display edge image in new window
	if(!im_edge.empty()) {
		if(show){
			namedWindow( "Display Image Edges", CV_WINDOW_AUTOSIZE );
			imshow( "Display Image Edges", im_edge );
		}
	} else
	    cout<<"Edge Empty ";
}

void IMG::Contour(bool show) {
	Scalar colorWhite( 255,255,255 );

	im_morph.create(im.size(), CV_8U);
//	im_plate.create(im.size(), CV_8U);
//	im_justPlate.create(im.size(), CV_8U);

	if(im_edge.empty()){
		Edge(false);
	}

	contourIm(im_edge, im_morph, im, im_justPlate);
	cout <<  "Contour: " << im_morph.size() << " Channels: " << im_morph.channels() << endl ;
	cout <<  "Plate: " << im_plate.size() << " Channels: " << im_plate.channels() << endl ;
	cout <<  "Just Plate: " << im_justPlate.size() << " Channels: " << im_justPlate.channels() << endl ;

	if(!im_morph.empty()) {
		if(show){
			namedWindow( "Morphological Image", CV_WINDOW_AUTOSIZE );
			imshow( "Morphological Image", im_morph );
			namedWindow( "Plate Image", CV_WINDOW_AUTOSIZE );
			imshow( "Plate Image", im_plate );
			namedWindow( "Just Plate Image", CV_WINDOW_AUTOSIZE );
			imshow("Just Plate Image", im_justPlate);

		}
	} else
	    cout<<"Morph Empty ";
}


IMG::~IMG() {
	// TODO Auto-generated destructor stub
}

void IMG::toGrayScale(Mat src, Mat dst){
	cvtColor( src, dst ,CV_BGR2GRAY);
	fastNlMeansDenoising(dst, dst, 3, 21,21);
	GaussianBlur( dst, dst, Size( 3,3 ), 0, 0 );


}

void IMG::toBinary(Mat src, Mat dst, int bthreshold){
	threshold( src, dst, bthreshold, max_value, CV_THRESH_BINARY );
	GaussianBlur( dst, dst, Size( 3,3 ), 0, 0 );


}

void IMG::edgeDetection(Mat src, Mat dst, int cthreshold){
	int dilation_type = 0;
	int dilation_size = 1;
	Mat element = getStructuringElement( dilation_type,
	                                       Size( 2*dilation_size + 1, 2*dilation_size + 1),
	                                       Point( dilation_size, dilation_size ) );

	Canny( src, dst, 100, 200,3);//500); //cthreshold, max_value, 3 );
//	erode(dst,dst,element);
	dilate(dst, dst, element);

}

void IMG::contourIm(const Mat& src, Mat& dst, const Mat& original, Mat& justPlate){
	vector< vector<Point> > contours;

	//	vector<vector<Point> > parallelContours;	//newly added
	vector<Vec4i> hierarchy;
	vector<Point> approx;

	Scalar colorBlack( 0, 0, 0 );
	Scalar colorWhite( 255, 255, 255 );

	// Assume license plate contour area is greater than 2% of the image size
	int mintresholdArea = src.rows*src.cols*0.005;
	int maxtresholdArea = src.rows*src.cols*0.05;
	int minthresholdPerm = 100;
	int maxthresholdPerm = 800;

	double minPlateRatio = 0.35;
	double maxPlateRatio = 1.0;

	Rect tempRect;

	int saveContour = 0;
	Mat tempIm = src.clone();

	findContours( tempIm, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

	//extracting contour from original image
	Mat mask = Mat::zeros(original.size(),CV_8UC1);

	cout <<  "Contours size:" << contours.size() << endl ;
	for (size_t i = 0; i < contours.size(); i++){  //count parallel was added
		approxPolyDP(Mat(contours[i]), approx, 2, true);
		if (fabs(contourArea(contours[i]) > mintresholdArea)
			  && fabs(contourArea(contours[i]) < maxtresholdArea)
			  && (arcLength(contours[i],true) > minthresholdPerm)
			  && (arcLength(contours[i],true) < maxthresholdPerm)
			  && (contourChildren(hierarchy,i) > 4)
		      && (contourChildren(hierarchy,i) < 40)
			  && approx.size() >= 4 ){

			cout <<  "Approx size:" << approx.size() << endl ;
			drawContours( dst, contours, i, colorWhite, 4, 8, hierarchy);
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);
			normalize(mask.clone(),mask,0.0,255.0,CV_MINMAX,CV_8UC1);

			tempRect = boundingRect( Mat(contours[i]) );
			int h = tempRect.height;
			int w = tempRect.width;
			double ratio = h/(double)w;
			cout << "Contour Height: " << h << endl;
			cout << "Contour Width: " << w << endl;
			cout <<  "Contour Height & width: " << ratio << endl;
			cout << i << endl;
			cout << "Inner Contour :" << hierarchy[i][2] << endl;

			if (isPlate(contours,i, im.rows) && ratio > minPlateRatio && ratio < maxPlateRatio)
				saveContour = i;
		}

	}

	imshow("Contour lines", dst);
	waitKey(0);

	original.copyTo(im_plate,mask);

	cout <<  "Saved contour:" << saveContour << endl ;

//	RotatedRect minRect(1);
//	minRect = minAreaRect(Mat (contours[saveContour]));

	Rect boundRect;
	boundRect = boundingRect( Mat(contours[saveContour]) );
	cout <<  "Bounding Rectangle: " << boundRect << endl ;

	justPlate = im_plate(boundRect); //boundRect

	// Increase the image size for integration
	resize( justPlate, im_plateBig, Size(), 1.5, 1.5, INTER_AREA);
}


/*

#if 0
	findContours( tempIm, contours, hierarchy, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	//  CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0,0)); //

	//extracting contour from original image
	Mat mask = Mat::zeros(original.size(),CV_8UC1);

	cout <<  "Contours size:" << contours.size() << endl ;
	for (size_t i = 0; i < contours.size(); i++){  //count parallel was added
		approxPolyDP(Mat(contours[i],true), approx, arcLength(Mat(contours[i]),true)*0.2, true);
		if ((fabs(contourArea(contours[i])) < 30000
				|| fabs(contourArea(contours[i])) > 31000)
			  && approx.size() != 4){

//			continue;
		}else{
			drawContours( dst, contours, i, colorBlack, CV_FILLED, 8, hierarchy);
			drawContours(mask, contours, i, Scalar(255), CV_FILLED);
			normalize(mask.clone(),mask,0.0,255.0,CV_MINMAX,CV_8UC1);
			saveContour = i;
		}

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//			contourImage.at<unit8_t>(p) = 0;//normal;  //start of new; replace if not working with old
//			approxPolyDP(Mat(contours[i],true), approx, arcLength(Mat(contours[i]),true)*0.2, true);
//			parallelContours[count]=contours[i];
//			count += 1;

//			if(isParallelIm(contourImage)){
	//			parallelContours[countParallel] = contours[i];
//			}
//		}
/*
	fillConvexPoly(contourImage, &approx[0], approx.size(),255,8,0);
//	for (size_t i = 0; i < approx.size(); i++)
//		drawContours(contourImage, approx, i , 255);
	Mat  mask(src.size(), CV_8UC3, normal);
	original.copyTo(mask, contourImage);
		imshow("Contours",contourImage);
*////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	}
/*
	original.copyTo(im_plate,mask);

	cout <<  "Saved contour:" << contourArea(contours[saveContour]) << endl ;

	Rect boundRect;
	boundRect = boundingRect( Mat(contours[saveContour]) );
	cout <<  "Bounding Rectangle: " << boundRect << endl ;

	justPlate = im_plate(boundRect);
#endif

*/

int IMG::contourChildren(vector<Vec4i>& hierachy, int index)
      {
         //first child
		 index = hierachy[index][2];
         if (index < 0)
            return 0;

         int count = 1;
         while (hierachy[index][0] > 0)
         {
            count++;
            index = hierachy[index][0];
         }
         return count;
      }

bool IMG::isPlate(vector <vector<Point>>& contour, int index,int sizeI){

	int totalCount = 0;
	int whiteColour = 0;
	int blueColour = 0;

	string indexS = to_string(index);

	double thresholdColourRatio = 0.007;
	double colourRatio = 0.0;

	int lMost = sizeI, tMost = sizeI;
	int rMost = 0, bMost = 0;
	int height = 0, width = 0;
	int i;

	double maxRatio = 1.0;
	double minRatio = 0.35;

	bool result = false;

	for(i = 0; i < contour[index].size(); i++){
		if (contour[index][i].x < lMost)
			lMost = contour[index][i].x;
		if (contour[index][i].x > rMost)
			rMost = contour[index][i].x;
		if (contour[index][i].y < tMost)
			tMost = contour[index][i].y;
		if (contour[index][i].y > bMost)
			bMost = contour[index][i].y;
	}

	height = bMost - tMost;
	width = rMost - lMost;

	double ratio = height/(double)width;



//	if (ratio <= maxRatio && ratio >= minRatio)
//			result = true;

	Rect boundRect;
	boundRect = boundingRect( Mat(contour[index]) );


	Mat im_tt = im_hsv(boundRect).clone();


	int h,s,v;
	for(int j = 0; j < im_tt.rows; j++){
//		Vec3b* colour = im_tt.ptr<cv::Vec3b>(j);
		for(int k = 0; k < im_tt.cols; k++){
			Vec3b colour = im_tt.at<Vec3b>(j, k);
			h = colour[0];
			s = colour[1];
			v = colour[2];
			totalCount++;
			if(h > 150  && s < 120 && v < 120){
				whiteColour++;
			}
			if(h < 190 && s > 160 && v < 140){
				blueColour++;
			}
//			cout <<"x: " << j << " y: " << k << " " << h << " " << s << " " << v << endl;
		}
	}
	imshow(indexS, im_tt);

	colourRatio = (blueColour+whiteColour)/(double) totalCount;
	cout << "COLOUR RATIO: " << blueColour << " " << whiteColour << " " << totalCount << " " << colourRatio << endl;
	cout << "HWR: " << height << " " << width << " " << ratio;

	if (colourRatio >= thresholdColourRatio && ratio <= maxRatio && ratio >= minRatio)
		result = true;

	return(result);
}

bool IMG::fileExists(string fName) {
	struct stat buf;
	return (stat(fName.c_str(), &buf) == 0);
}





