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

Mat normalize(const Mat& mat) {
	
	Mat im_gray;
	cvtColor(mat, im_gray, COLOR_BGR2GRAY);

	Mat im_bw;
	threshold(im_gray, im_bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	Mat hist_v = Mat::zeros(im_bw.cols, 1, CV_32SC1);
	Mat hist_h = Mat::zeros(im_bw.rows, 1, CV_32SC1);
	for (int i = 0; i < im_bw.rows; i++) {
		// pointer to acces image's row this is faster than at template function
		uchar *im_row_ptr = im_gray.ptr<uchar>(i);

		for (int j = 0; j < im_bw.cols; j++) {
			hist_h.at<int>(i) += im_row_ptr[j];
			hist_v.at<int>(j) += im_row_ptr[j];
		}
	}

	//imshow("Gray", im_gray);
	//imshow("Bw", im_bw);

	int top = -1;
	int bottom = -1;
	for (int i = 0; i < im_bw.cols; ++i) {
		if (hist_v.at<int>(i) > 20) {
			bottom = i + 1;
			if (top == -1) {
				top = i - 1;
			}
		}
	}

	cout << "Top: " <<  top << " | " << "Bottom: " << bottom << endl;



	return im_bw;
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

int main()
{

	
	// normalize(mat);

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

	for (string fileName : getFilesName(path.c_str(), ".png")) {
		Mat mat = loadImage(path + fileName);

		cvtColor(mat, mat, CV_BGR2GRAY);
		stringstream humoments;
		for (auto d : huMomentFeature(mat)) {
			humoments << d << ",";
		}

		humoments << fileName.substr(0, fileName.find_first_of('_'));
		
		cout << "Adding : " << humoments.str() << endl;
		manager.addDatas(humoments.str(), f);
	}
	
	

	waitKey(1);
	system("pause");
    return 0;
}

