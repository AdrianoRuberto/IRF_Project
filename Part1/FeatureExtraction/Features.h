
#include <cv.h>
#include <highgui.h>

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

	/*
	Returns the moments feature.
	This feature is RST variant!
	
	@param mat Image matrix
	@return Moments vector containing 24 features
	*/
	static vector<double> MomentFeature(const Mat& mat) {
		std::vector<double> res;

		cv::Moments moments = cv::moments(mat);

		// spatial moments
		res.push_back(moments.m00);
		res.push_back(moments.m10);
		res.push_back(moments.m01);
		res.push_back(moments.m20);
		res.push_back(moments.m11);
		res.push_back(moments.m02);
		res.push_back(moments.m30);
		res.push_back(moments.m21);
		res.push_back(moments.m12);
		res.push_back(moments.m03);
		// central moments
		res.push_back(moments.mu20);
		res.push_back(moments.mu11);
		res.push_back(moments.mu02);
		res.push_back(moments.mu03);
		res.push_back(moments.mu21);
		res.push_back(moments.mu12);
		res.push_back(moments.mu03);
		// central normalized moments
		res.push_back(moments.nu20);
		res.push_back(moments.nu11);
		res.push_back(moments.nu02);
		res.push_back(moments.nu30);
		res.push_back(moments.nu21);
		res.push_back(moments.nu12);
		res.push_back(moments.nu03);
		
		return res;
	}

	/*
	Returns the center of mass in relative position (in an interval
	between 0 and 1).
	
	@param mat the matrix
	@return center of mass
	*/
	static vector<double> RelCenterOfMass(const Mat& mat) {
		vector<double> ret;

		int count = 0;

		Mat bin;

		threshold(mat, bin, 128, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

		int x = 0;
		int y = 0;

		for (int i = 0; i<bin.rows; ++i) {
			for (int j = 0; j<bin.cols; ++j) {
				if (bin.at<uchar>(i,j) == 0) {
					count++;
					x += j;
					y += i;
				}
			}
		}

		if (count == 0) {
			ret.push_back(0);
			ret.push_back(0);
		}
		else {
			ret.push_back(((1.0 * x) / count) / bin.cols);
			ret.push_back(((1.0 * y) / count) / bin.rows);
		}

		return ret;
	
	}

	/*
	Returns the length and area of a convex hull fitted to the image
	
	@param mat the matrix
	@return vector with length and area of the convex hull
	*/
	static vector<double> ConvexHull(const Mat& mat) {
		//conf
		int thresh = 100;
		
		// blur matrice
		blur(mat, mat, Size(3, 3));

		Mat threshold_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		/// Detect edges using Threshold
		threshold(mat, threshold_output, thresh, 255, THRESH_BINARY);

		/// Find contours
		findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		/// Find the convex hull object for each contour
		vector<vector<Point> > hull(contours.size());
		for (int i = 0; i < contours.size(); i++)
		{
			convexHull(Mat(contours[i]), hull[i], false);
		}

		int lengthOfHull = 0;
		int areaOfHull = 0;
		int currentArea, currentArcLength;
		for (int i = 0; i< contours.size(); i++) {
			// search for biggest Arc
			currentArcLength = arcLength(hull[i], true);
			if (currentArcLength > lengthOfHull) {
				lengthOfHull = currentArcLength;
				// store area for normalization
				areaOfHull = contourArea(hull[i], false);
			}
		}
		
		vector<double> ret;

		ret.push_back(lengthOfHull);
		ret.push_back(areaOfHull);

		return ret;
	}

	/*
	Returns connected components in image
	Does not work right :-(
	
	@param mat image matrix
	@return number of connected components
	*/
	static int ConnectedComponentsFeature(const Mat& mat) {
		
		// blur slightly and threshold
		Mat im_blur;
		blur(mat, im_blur, Size(20, 20));
		Mat im_bw;
		Scalar average = mean(im_blur);
		threshold(mat, im_bw, average.val[0], 255, THRESH_BINARY);

		// extract contours
		Mat dilated;
		morphologyEx(im_bw, dilated, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
		vector<vector<Point> > contours;
		findContours(dilated, contours, RETR_LIST, CV_CHAIN_APPROX_TC89_KCOS);


		//std::cout << contours.size() << endl;
		//namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
		//cvtColor(dilated, dilated, COLOR_GRAY2BGR);
		//drawContours(dilated, contours, -1, Scalar(0,255,0));
		//imshow("Display window", dilated);
		//waitKey(0);

		return contours.size();
	}



	Features();
	~Features();
};

