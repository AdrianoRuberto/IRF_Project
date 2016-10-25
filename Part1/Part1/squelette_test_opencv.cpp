//////////////////////////////////////////////////////////////////////////
// Module IRF, 5-iNFO
// Projet, séance 1
// thème : premier pas en OpenCV
// contenu : charge, affiche, réduction, calcul et affichage d'histogramme
// auteur : Luce Morin
// date : 4/10/2010
//////////////////////////////////////////////////////////////////////////

#define _USE_MATH_DEFINES

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
	HoughLinesP(im, lines, 1, CV_PI / 180, 50, 50, 10);
	return lines;
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
	//imshow("Reduced", imreduite);

	//Grayscale matrix
	Mat grayscaleMat(imreduite.size(), CV_8U);

	//Convert BGR to Gray
	cvtColor(imreduite, grayscaleMat, CV_BGR2GRAY);

	// Clean the image
	Mat dst = cleanImg(grayscaleMat);
	//imshow("Clean img", dst);

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
