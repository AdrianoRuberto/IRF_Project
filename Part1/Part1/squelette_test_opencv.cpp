//////////////////////////////////////////////////////////////////////////
// Module IRF, 5-iNFO
// Projet, séance 1
// thème : premier pas en OpenCV
// contenu : charge, affiche, réduction, calcul et affichage d'histogramme
// auteur : Luce Morin
// date : 4/10/2010
//////////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES

#define IMG_HOR 2480 / 5
#define IMG_VER 3508 / 5

typedef vector<vector<Vec4i>> VectorHV;

#include "histogram.h"

#include "cv.h"
#include "highgui.h"
using namespace cv;

#include <iostream>
#include <cmath>

using namespace std;

/*
	Gets the lines on a given image.
	@param im	The image
	@return The lines founded
*/
vector<Vec4i> getLines(const Mat& im) {
	vector<Vec4i> lines;
	vector<Vec4i> cleanedLines;

	size_t estiSizeHor = IMG_HOR * 261 / 2480;
	size_t estiSizeVer = IMG_VER * 258 / 3508;
	size_t upper = max(estiSizeHor, estiSizeVer);
	size_t upper_sqr = upper * upper * 1.1 * 1.1;
	size_t estiGap = min(IMG_HOR * 80 / 2480, IMG_VER * 85 / 3508) * 0.9;

	const size_t LOWER_BOUND = 0;
	const size_t UPPER_BOUND = upper_sqr;

	HoughLinesP(im, lines, 1, CV_PI / 180, 70, upper * 0.95, estiGap);

	// Remove bad lines
	for (size_t i = 0; i < lines.size(); ++i) {
		Vec4i l = lines[i];
		int dist = (l[2] - l[0]) * (l[2] - l[0]) + (l[3] - l[1]) * (l[3] - l[1]);
		if (dist <= UPPER_BOUND) {
			cleanedLines.push_back(l);
		}
	}

	cout << lines.size() << " | " << cleanedLines.size() << " (" << lines.size() - cleanedLines.size() << ")" << endl;

	return cleanedLines;
}

Point2f computeIntersect(cv::Vec4i a, cv::Vec4i b)
{
	int x1 = a[0], y1 = a[1], x2 = a[2], y2 = a[3];
	int x3 = b[0], y3 = b[1], x4 = b[2], y4 = b[3];
	if (float d = ((float)(x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4)))
	{
		cv::Point2f pt;
		pt.x = ((x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4)) / d;
		pt.y = ((x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4)) / d;
		//-10 is a threshold, the POI can be off by at most 10 pixels
		if (pt.x<min(x1, x2) - 10 || pt.x>max(x1, x2) + 10 || pt.y<min(y1, y2) - 10 || pt.y>max(y1, y2) + 10) {
			return Point2f(-1, -1);
		}
		if (pt.x<min(x3, x4) - 10 || pt.x>max(x3, x4) + 10 || pt.y<min(y3, y4) - 10 || pt.y>max(y3, y4) + 10) {
			return Point2f(-1, -1);
		}
		return pt;
	}
	else
		return cv::Point2f(-1, -1);
}

/*
 0 : hori
 1 : verti
*/
VectorHV linesToHV(const vector<Vec4i>& lines) {
	vector<Vec4i> horizontals, verticals;

	const size_t HORIZONTAL_BOUND = 2;
	const size_t VERTICAL_BOUND = 2;

	for (size_t i = 0; i < lines.size(); ++i) {
		Vec4i l = lines[i];
		if (abs(l[1] - l[3]) <= HORIZONTAL_BOUND)
		{
			horizontals.push_back(l);
		}
		else if (abs(l[0] - l[2]) <= VERTICAL_BOUND) {
			verticals.push_back(l);
		}
	}

	VectorHV res;
	res.push_back(horizontals);
	res.push_back(verticals);

	return res;
}

/*
	Shows the lines on an image.
	@param source	The image used for the lines
	@param lines	The lines
*/
void showLines(const Mat& source, const vector<Vec4i>& lines) {
	static int j = 0;
	Mat cdst;
	cvtColor(source, cdst, COLOR_GRAY2BGR);
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		line(cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 0, 255), 2, CV_AA);
	}

	imshow("Lines " + to_string(j++), cdst);
}

/*
	Loads the image.
	If the image can't be loaded, exit the program
	@param name Name of the image to load
	@return The loaded image
*/
Mat loadImage(const string& name) {
	//charge et affiche l'image 
	Mat im = imread(name);
	if (im.data == NULL) {
		cerr << "Image not found: " << name << endl;
		exit(0);
	}
	return im;
}

/*
	Cleans the image.
	@param The binary image
	@return The cleaned image
*/
Mat cleanImg(const Mat& binaryImg) {
	Mat dst;
	GaussianBlur(binaryImg, dst, Size(3, 3), 0);
	adaptiveThreshold(dst, dst, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 75, 10);
    bitwise_not(dst, dst);
	return dst;
}

/*
	Reduces the image of a given reduction
	@param im			The image to reduce
	@param reduction	The reduction to apply

*/
Mat reduce(const Mat& im, int reduction) {

	Size tailleReduite(im.cols / reduction, im.rows / reduction);
	Mat imreduite = Mat(tailleReduite, CV_8UC3); //cree une image à 3 canaux de profondeur 8 bits chacuns
	resize(im, imreduite, tailleReduite);

	return imreduite;
}

void slice(const Mat& im) {
	// Reduce de image size
	Mat imreduite = reduce(im, 5);

	//Grayscale matrix
	Mat grayscaleMat(imreduite.size(), CV_8U);

	//Convert BGR to Gray
	cvtColor(imreduite, grayscaleMat, CV_BGR2GRAY);

	// Clean the image
	Mat dst = cleanImg(grayscaleMat);
	//imshow("Clean img", dst);

	VectorHV lines = linesToHV(getLines(dst));

	for (size_t i = 0; i < lines[0].size(); ++i) {
		for (size_t j = 0; j < lines[1].size(); ++j) {
			computeIntersect(lines[0][i], lines[1][j]);
		}
	}
	

	// Show the lines founded on dst
	showLines(dst, getLines(dst));
}

int main (void) {


	

	// Load the image
	slice(loadImage("00000.png"));
	slice(loadImage("00000_a.png"));
	slice(loadImage("00000_b.png"));


	

	//computeHistogram("histogramme", im);

	//termine le programme lorsqu'une touche est frappee
	waitKey(0);
}
