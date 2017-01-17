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


double nbBlackPixel(const Mat& mat) {

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

string getClassName() {
	stringstream ss;
	ss << "{";
	for (int i = 0; i < NAMES.size(); ++i) {
		ss << NAMES.at(i) << (i < NAMES.size() - 1 ? "," : "}");
	}
	return ss.str();
}

string getLabelName(const string& fileName) {
	ifstream file(path.c_str() + fileName.substr(0, fileName.find_last_of(".")) + ".txt");
	string s, label;
	while (file >> s >> label)
	{
		const char* ptr = strstr(s.c_str(), "label");
		if (ptr != NULL) {
			if (std::find(NAMES.begin(), NAMES.end(), label) != NAMES.end()) {
				return label;
			}
			else {
				exit(fileName + " doesn't have a right label : " + label);
			}
		}
	}
}

/*
Process the given fileName with different features.

@param fileName The fileName
@param manager	The ARFF manager
*/
string process(const string& fileName, ARFFManager& manager) {

	Mat mat = loadImage(path + fileName);

	cvtColor(mat, mat, CV_BGR2GRAY);

	stringstream data;

	/*
	for (int i = 1; i <= 7; ++i) {
	manager.addAttribute({ "M" + to_string(i) , "NUMERIC" });
	}
	*/
	/*
	for (auto hu : huMomentFeature(mat)) {
	data << hu << ",";
	}
	*/

	manager.addAttribute({ "RatioPixel", "NUMERIC" });
	data << (double)nbBlackPixel(mat) / (mat.rows * mat.cols) << ",";

	data << getLabelName(fileName);
	return data.str();
}

int main()
{
	string arffName = "test.arff";
	ofstream f(arffName);
	ARFFManager manager("IRF_Project");

	vector<string> fileNames;

	for (string s : getFilesName(path.c_str(), ".png")) {
		fileNames.push_back(s);
	}

	cout << "Processing all files (" << fileNames.size() << ")" << endl << "..." << endl;

	while (fileNames.size() > 0) {
		int nb = rand() % fileNames.size();

		string fileName = fileNames.at(nb);

		manager.addDatas(process(fileName, manager));

		fileNames.erase(fileNames.begin() + nb);
	}	

	cout << "Successfuly process all files" << endl;


	manager.addAttribute({ "class", getClassName() });
	cout << "Writting file" << endl;
	cout << "..." << endl;
	manager.write(f);

	cout << "File " << arffName << " finished to be written" << endl;

	f.close();
	waitKey(0);
    return 0;
}

