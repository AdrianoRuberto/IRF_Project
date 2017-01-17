// FeatureExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ARFFManager.h"
#include "dirent.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

string path = "..//Part1//Results/";
vector<string> NAMES = { "accident", "bomb", "car", "casualty", "electricity", "fire", "fire_brigade", "flood", "gas", "injury", "paramedics", "person", "police", "roadblock"};


void exit(string exitMsg) {
	cerr << exitMsg << endl;
	system("pause");
	exit(0);
}
/*
Loads the image.
If the image can't be loaded, exit the program
@param name Name of the image to load
@return The loaded image
*/
Mat loadImage(const string &name) {
	Mat im = imread(name);
	if (im.data == NULL) {
		exit("Image not found: " + name);
	}
	return im;
}

vector<string> getFilesName(const char* path, const char* filter) {
	vector<string> names;
	DIR* rep = opendir(path);
	struct dirent* fileRead = readdir(rep);
	while ((fileRead = readdir(rep)) != NULL) {
		if (strstr(fileRead->d_name, filter) != NULL) {
			names.push_back(fileRead->d_name);
		}
	}
	
	return names;
}

string getNormalizeNameFromFile(const string& path) {
	string fileTxt = path.substr(0, path.size() - 3) + "txt"; 
	ifstream file(fileTxt);
	
	if (!file.is_open()) {
		exit("File not found: " + fileTxt);
	}

	string line;
	while (getline(file, line)) {
		if (strstr(line.c_str(), "label") != NULL) {
			file.close();
			string s = line.substr(6, line.size());
			return line.substr(6, line.size());
		}
	}
	file.close();

	exit("Couldn't get any label");
}

/*
cuts away white space around image an returns a grayscale image

@param mat  image matrix
@return cropped and grayscaled image
*/
Mat normalize(const Mat& mat) {
	
	// Threshold the HSV image, keep only the blue/colourful pixels
	Mat hsv;
	Scalar hsv_l(20, 60, 60);
	Scalar hsv_h(130, 255, 255);
	cvtColor(mat, hsv, CV_BGR2HSV);
	Mat bw;
	inRange(hsv, hsv_l, hsv_h, bw);
	
	// convert to grayscale
	Mat im_gray;
	cvtColor(mat, im_gray, COLOR_BGR2GRAY);

	// blur slightly and threshold
	Mat im_blur;
	blur(bw, im_blur, Size(10, 10));
	Mat im_bw;
	Scalar average = mean(im_blur);
	threshold(im_blur, im_bw, average.val[0], 255, THRESH_BINARY);

	// extract contours
	Mat dilated;
	morphologyEx(im_bw, dilated, MORPH_OPEN, getStructuringElement(MORPH_ELLIPSE, Size(10, 10)));
	vector<vector<Point> > contours;
	findContours(dilated, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

	// strip large contour areas
	vector<vector<Point> > stripped_contours;
	int image_size = im_bw.cols * im_bw.rows;
	for (auto c : contours) {
		if (contourArea(c) < image_size) {
			stripped_contours.push_back(c);
		}
	}
	

	// find best bounding box
	int best_box[4] = { -1, -1, -1, -1 };
	for (auto c : stripped_contours) {
		Rect bound = boundingRect(c);
		int x = bound.x;
		int y = bound.y;
		int x2 = bound.x + bound.width;
		int y2 = bound.y + bound.height;
		if (best_box[0] < 0) {
			best_box[0] = x;
			best_box[1] = y;
			best_box[2] = x2;
			best_box[3] = y2;
		}
		else {
			if (bound.x < best_box[0]) {
				best_box[0] = bound.x;
			}
			if (bound.y < best_box[1]) {
				best_box[1] = bound.y;
			}
			if (x2 > best_box[2]) {
				best_box[2] = x2;
			}
			if (y2 > best_box[3]) {
				best_box[3] = y2;
			}
		}
	}

	Rect myROI(best_box[0], best_box[1], best_box[2] - best_box[0] , best_box[3] - best_box[1]);

	// Crop the full image to that image contained by the rectangle myROI
	// Note that this doesn't copy the data
	Mat croppedImage = im_gray(myROI);
	Mat output;
	croppedImage.copyTo(output);

	cout << "Top: " << best_box[1] << " | " << "Bottom: " << best_box[3] << endl;
	
	return output;
}

/*
Returns the number of circle
This feature is Rotation variant.

@param Mat The matrice
@return The aspect/ratio
*/
int getNumberOfCircle(const Mat& mat) {
	vector<Vec3f> circles;
	HoughCircles(mat, circles, CV_HOUGH_GRADIENT, 1, mat.rows / 8, 150, 50, 0, 0);

	return circles.size();
}


double area(const Mat& mat) {
	

}

int getNumberOfSquare(const Mat& mat) {

}

/*
Returns the aspect Ratio feature.
This feature is Rotation variant.

@param Mat The matrice
@return The aspect/ratio
*/
double aspectRatio(const Mat& mat) {
	// Rotation variant feature
	return (double)(mat.cols + 1) / (double)(mat.rows + 1);
}

/*
Returns the Hu Moments feature.
This feature is RST invariant.

@param Mat The matrice
@return the Hu Moments feature.
*/
vector<double> huMomentFeature(const Mat& mat) {
	double humoments[7];
	HuMoments(moments(mat), humoments);

	vector<double> res;
	for (int i = 0; i < 7; ++i) {
		res.push_back(humoments[i]);
	}

	return res;
}

void test_normalize(String file) {
	Mat mat = loadImage(path + file);
	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", normalize(mat));
	waitKey(0);
}

int main()
{
	/*test_normalize("roadblock_026_21_7_3.png");

	test_normalize("accident_000_00_1_2.png");
	test_normalize("gas_008_03_7_3.png");
	test_normalize("injury_017_13_4_3.png");
	test_normalize("paramedics_032_14_2_4.png");
	test_normalize("police_013_09_2_4.png");
	test_normalize("bomb_018_07_7_4.png");*/


	/*
	
	map<string, vector<Mat>> files;
	for (string fileName : getFilesName(path.c_str(), ".png")) {
		string key = getNormalizeNameFromFile(path + fileName);
		if (files.find(key) == files.end()) {
			cout << "Creating new key for " << key << endl;
			files.insert(files.begin(), pair<string, vector<Mat>>(key, vector<Mat>()));
		}
		cout << "Adding : " << fileName << endl;
		files.at(key).push_back(loadImage(path + fileName));
	}
	*/


	ofstream f("test.arff");


	ARFFManager manager("IRF_Project", f);
	manager.addAttribute({ "M1", "NUMERIC" }, f);
	manager.addAttribute({ "M2", "NUMERIC" }, f);
	manager.addAttribute({ "M3", "NUMERIC" }, f);
	manager.addAttribute({ "M4", "NUMERIC" }, f);
	manager.addAttribute({ "M5", "NUMERIC" }, f);
	manager.addAttribute({ "M6", "NUMERIC" }, f);
	manager.addAttribute({ "M7", "NUMERIC" }, f);

	stringstream ss;
	ss << "{";
	for (int i = 0; i < NAMES.size(); ++i) {
		ss << NAMES.at(i) << (i < NAMES.size() - 1 ? "," : "}");
	}

	manager.addAttribute({ "class", ss.str() }, f);

	vector<string> fileNames;

	for (string s : getFilesName(path.c_str(), ".png")) {
		fileNames.push_back(s);
	}

	while (fileNames.size() > 0) {
		int nb = rand() % fileNames.size();

		string fileName = fileNames.at(nb);

		Mat mat = loadImage(path + fileName);

		cvtColor(mat, mat, CV_BGR2GRAY);
		stringstream humoments;
		for (auto d : huMomentFeature(mat)) {
			humoments << d << ",";
		}

		ifstream file(path.c_str() + fileName.substr(0, fileName.find_last_of(".")) + ".txt");
		string s, label;
		while (file >> s >> label)
		{
			const char* ptr = strstr(s.c_str(), "label");
			if (ptr != NULL) {
				humoments << label;
				cout << "Adding : " << humoments.str() << endl;
				manager.addDatas(humoments.str(), f);
				fileNames.erase(fileNames.begin() + nb);
				break;
			}
		}

	}	

	waitKey(1);
	system("pause");
    return 0;
}

