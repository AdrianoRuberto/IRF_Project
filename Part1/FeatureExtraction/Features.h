
#include <cv.h>

using namespace std;
using namespace cv;

#pragma once
class Features
{
public:

	/*
	Returns the number of circle
	This feature is Rotation variant.

	@param Mat The matrice
	@return The aspect/ratio
	*/
	static int getNumberOfCircle(const Mat& mat) {
		vector<Vec3f> circles;
		HoughCircles(mat, circles, CV_HOUGH_GRADIENT, 1, mat.rows / 8, 150, 50, 0, 0);

		return circles.size();
	}

	/*
	Returns the number of black pixel in the matrice

	@param Mat the matrice
	@return The number of black pixel
	*/
	static double nbBlackPixel(const Mat& mat) {
		int count = 0;

		Mat bin;

		threshold(mat, bin, 128, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

		for (int i = 0; i < bin.rows; ++i) {
			for (int j = 0; j < bin.cols; ++j) {
				if (bin.at<uchar>(i, j) == 0) {
					count++;
				}
			}
		}

		return count;
	}


	static double area(const Mat& mat) {


	}

	static int getNumberOfSquare(const Mat& mat) {

	}

	/*
	Returns the aspect Ratio feature.
	This feature is Rotation variant.

	@param Mat The matrice
	@return The aspect/ratio
	*/
	static double aspectRatio(const Mat& mat) {
		// Rotation variant feature
		return (double)(mat.cols + 1) / (double)(mat.rows + 1);
	}

	/*
	Returns the Hu Moments feature.
	This feature is RST invariant.

	@param Mat The matrice
	@return the Hu Moments feature.
	*/
	static vector<double> huMomentFeature(const Mat& mat) {
		double humoments[7];
		HuMoments(moments(mat), humoments);

		vector<double> res;
		for (int i = 0; i < 7; ++i) {
			res.push_back(humoments[i]);
		}

		return res;
	}



	Features();
	~Features();
};

