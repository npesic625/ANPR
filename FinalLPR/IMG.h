/*
 * IMG.h
 *
 *  Created on: 2016-02-29
 *      Author: npesic
 */

#ifndef IMG_H_
#define IMG_H_

using namespace std;
using namespace cv;

class IMG{

private:
	string fileName;
	Mat im,
		im_hsv,
		im_gray,
		im_binary,
		im_edge,
		im_morph,
		im_contour,
		im_rect,
		im_testplate,
		im_plate,
		im_plateBig,
		im_justPlate;

	void toGrayScale(Mat src, Mat dst);
	void toBinary(Mat src, Mat dst, int value);
	void edgeDetection(Mat src,Mat dst, int cthreshold);
	void contourIm(const Mat& src, Mat& dst, const Mat& original, Mat& subImg);

	int contourChildren(vector<Vec4i>& hierachy, int index);
	bool IMG::isPlate(vector <vector<Point>>& points,int index, int sizeI);


	bool fileExists(string fName);

public:

	IMG(string fName);
	virtual ~IMG();

	Mat getOrig(void);
	Mat getMaskedPlate(void);
	Mat getPlate(void);
	Mat getBigPlate(void);
	void GrayScale (bool show);
	void Binary (bool show);
	void Edge (bool show);
	void Contour (bool show);

};


#endif /* IMG_H_ */
