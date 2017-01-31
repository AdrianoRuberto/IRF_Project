// FeatureExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ARFFManager.h"
#include "Features.h"
#include "dirent.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <cv.h>
#include <highgui.h>

using namespace std;
using namespace cv;

const string path = "..//Part1//ResultsV2/";
const string arffName = "test.arff";
vector<string> NAMES = { "accident", "bomb", "car", "casualty", "electricity", "fire", "fire_brigade", "flood", "gas", "injury", "paramedics", "person", "police", "roadblock"};


/*
	Exits with a message
	@param exitMsg the message to show before exit
*/
void exit(const string& exitMsg) {
	cerr << exitMsg << endl;
	waitKey(1);
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


/*
	Gets all files name in the directory path.
	The filter should be the extension of the files to search.

	@param path		The path to the directory
	@param filter	The extension of files to find
	@return All the files founded
*/
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
	Cuts away white space around image an returns a grayscale image.

	@param mat  image matrix
	@return cropped and grayscaled image
*/
Mat normalize(const Mat& mat) {
	
	// Threshold the HSV image, keep only the blue/colourful pixels
	Mat hsv;
	Scalar hsv_l(20, 80, 80);
	Scalar hsv_h(130, 255, 255);
	cvtColor(mat, hsv, CV_BGR2HSV);
	Mat bw;
	inRange(hsv, hsv_l, hsv_h, bw);
	//namedWindow("hsv-filt", WINDOW_AUTOSIZE);// Create a window for display.
	//imshow("hsv-filt", bw);
	//waitKey(0);
	
	// convert to grayscale
	Mat im_gray;
	cvtColor(mat, im_gray, COLOR_BGR2GRAY);
	
	// blur slightly and threshold
	Mat im_blur;
	blur(bw, im_blur, Size(10, 10));
	Mat im_bw;
	Scalar average = mean(im_blur);
	threshold(im_blur, im_bw, average.val[0]+10, 255, THRESH_BINARY);

	// vector with all non-black point positions
	std::vector<cv::Point> nonBlackList;
	nonBlackList.reserve(im_bw.rows*im_bw.cols);

	// add all non-black points to the vector
	//TODO: there are more efficient ways to iterate through the image
	for (int j = 0; j<im_bw.rows; ++j)
		for (int i = 0; i<im_bw.cols; ++i)
		{
			// if not black: add to the list
			if (im_bw.at<uchar>(j, i) != 0)
			{
				nonBlackList.push_back(cv::Point(i, j));
			}
		}

	int size = nonBlackList.size();
	if (size < 200) {
		//namedWindow("src", WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("src", im_gray);
		//namedWindow("hsv-filt", WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("hsv-filt", bw);
		//namedWindow("thresh", WINDOW_AUTOSIZE);// Create a window for display.
		//imshow("thresh", im_bw);
		//waitKey(0);
		
		//we have a bad picture, just take the whole one
		Mat output;
		im_gray.copyTo(output);

		return output;
	}
	// create bounding rect around those points
	cv::Rect bb = cv::boundingRect(nonBlackList);

	// display result
	//namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	//cv::imshow("found rect", im_gray(bb));
	//waitKey(0);

	Mat output;
	im_gray(bb).copyTo(output);
	
	return output;
}

/*
	Gets the class name for the data
	@return a string separeted with ',' in between each class name
*/
string getClassName() {
	stringstream ss;
	ss << "{";
	for (int i = 0; i < NAMES.size(); ++i) {
		ss << NAMES.at(i) << (i < NAMES.size() - 1 ? "," : "}");
	}
	return ss.str();
}

/*
	Gets the label name of a image. If the label is not found, exit.
	@param fileName	The file name 
	@return the label name
*/
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
	Gets a zoning matrices.
	@param mat		the matrice to zone
	@param nbCol	the number of column
	@param nbRow	the number of row
*/
vector<Mat> getZoningMatrices(const Mat& mat, int nbCol, int nbRow) {
	if (nbCol <= 0 || nbRow <= 0) {
		throw out_of_range("NbCol or nbLine is out of range");
	}

	vector<Mat> matrices;

	int colStep = mat.cols / nbCol;
	int rowStep = mat.rows / nbRow;

	for (int col = 0; col + colStep <= mat.cols; col += colStep) {
		for (int row = 0; row + rowStep <= mat.rows; row += rowStep) {
			Mat m = mat(Range(row, row + rowStep), Range(col, col + colStep));
			matrices.push_back(m);
		}
	}

	return matrices;
}

/*
Process the given fileName with different features.

@param fileName The fileName
@param manager	The ARFF manager
*/
string process(const string& fileName, ARFFManager& manager) {

	Mat mat = normalize(loadImage(path + fileName));

	stringstream data;
	
	vector<Mat> zones = getZoningMatrices(mat, 2, 2);

	manager.addAttribute({ "AspectRatio", "NUMERIC" });
	data << Features::aspectRatio(mat) << ",";

	manager.addAttribute({ "HullLength/Area", "NUMERIC" });
	vector<double> hla = Features::ConvexHull(mat);
	data << hla[0] / hla[1] << ",";

	for (int j = 1; j <= 24; ++j) {
		manager.addAttribute({ "Moment_" + to_string(j) , "NUMERIC" });
	}

	for (auto m : Features::MomentFeature(mat)) {
		data << m << ",";
	}

	for (int j = 1; j <= 7; ++j) {
		manager.addAttribute({ "HUMoment_" + to_string(j) , "NUMERIC" });
	}

	for (auto hu : Features::huMomentFeature(mat)) {
		data << hu << ",";
	}
	
	manager.addAttribute({ "ConnectedComponents", "NUMERIC" });
	data << Features::ConnectedComponentsFeature(mat) << ",";

	manager.addAttribute({ "NumberOfCircles", "NUMERIC" });
	data << Features::NumberOfCircles(mat) << ",";

	manager.addAttribute({ "NumberOfLines", "NUMERIC" });
	data << Features::NumberOfLines(mat) << ",";

	//Features::HarrisCorners(mat);

	vector<double> com = Features::RelCenterOfMass(mat);
	manager.addAttribute({ "RelCenterOfMassX", "NUMERIC" });
	manager.addAttribute({ "RelCenterOfMassY", "NUMERIC" });
	data << (double)com[0] << "," << (double)com[1] << ",";
	manager.addAttribute({ "RatioPixel", "NUMERIC" });
	data << (double)Features::nbBlackPixel(mat) / (mat.rows * mat.cols) << ",";
	

	for (int i = 0; i < zones.size(); ++i) {
		mat = zones.at(i);

		vector<double> com = Features::RelCenterOfMass(mat);
		manager.addAttribute({ "RelCenterOfMassX_" + to_string(i), "NUMERIC" });
		data << (double)com[0] << ",";
		manager.addAttribute({ "RelCenterOfMassY_" + to_string(i), "NUMERIC" });
		data << (double)com[1] << ",";
		

		manager.addAttribute({ "RatioPixel_" + to_string(i), "NUMERIC" });
		data << (double)Features::nbBlackPixel(mat) / (mat.rows * mat.cols) << ",";

	}



	data << getLabelName(fileName);
	return data.str();
}

/*
	Draws a progression bar
	@progress the progresse [0, 1]
*/
void drawProgressionBar(float progress) {
	int barWidth = 50;

	std::cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) std::cout << "=";
		else if (i == pos) std::cout << ">";
		else std::cout << " ";
	}
	std::cout << "] " << int(progress * 100.0) << " %\r";
	std::cout.flush();
}

int main()
{
	ofstream f(arffName);
	ARFFManager manager("IRF_Project");

	vector<string> fileNames;

	for (string s : getFilesName(path.c_str(), ".png")) {
		fileNames.push_back(s);
	}

	const int NB_FILES = fileNames.size();
	cout << "Processing all files (" << fileNames.size() << ")" << endl << "..." << endl;

	while (fileNames.size() > 0) {
		int nb = rand() % fileNames.size();

		string fileName = fileNames.at(nb);

		manager.addDatas(process(fileName, manager));

		fileNames.erase(fileNames.begin() + nb);
		drawProgressionBar((float)(NB_FILES - fileNames.size()) / NB_FILES);
	}	

	cout << "Successfuly process all files" << endl;


	manager.addAttribute({ "class", getClassName() });
	cout << "Writting file" << endl;
	cout << "..." << endl;
	manager.write(f);

	cout << "File " << arffName << " finished to be written" << endl;

	f.close();
	system("pause");
    return 0;
}

