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
vector<string> NAMES = { "accident", "bomb", "car", "casualty", "electricity", "fire", "fire brigade", "flood", "gas", "injury", "paramedics", "person", "police", "road block"};


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

int main()
{
	Mat mat = loadImage(path + "accident_000_00_1_2.png");
	normalize(mat);
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




	ARFFManager manager("IRF_Project");
	manager.addAttribute({ "LCC", "string" });
	manager.addAttribute({ "sepallength", "NUMERIC" });

	stringstream ss;
	ss << "{";
	for (int i = 0; i < NAMES.size(); ++i) {
		ss << NAMES.at(i) << (i < NAMES.size() - 1 ? "," : "}");
	}

	manager.addAttribute({ "class", ss.str() });
	
	manager.addDatas("");

	ofstream f("test.arff");

	manager.write(f);
	*/
	system("pause");
    return 0;
}

