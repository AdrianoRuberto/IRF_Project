//////////////////////////////////////////////////////////////////////////
// Module IRF, 5-iNFO
// Projet - première étape
// thème : Première étape du projet
// contenu : Pre-processing and image processing
// auteur : 
// date : TODO
//////////////////////////////////////////////////////////////////////////

//#include"stdafx.h"
#include "histogram.h"
#include "cv.h"
#include "highgui.h"
#include "dirent.h"
#include <iostream>
#include <fstream>
#include <math.h>
#include <iomanip>
#include <array>
#include <time.h>

using namespace cv;
using namespace std;

// upper band to be removed in the input images
#define NOISE_BAND_FRAC 20

// threshold for selecting line in the horizontal and vertical directions
#define LINE_THRESHOLD 40

// threshold for the size of found rectangles
#define RECT_THRESHOLD 68

// offset to apply to lines coordinates to avoid taking the borders
#define COORD_OFFSET 8

// Divide the image by 2 according to the vertical direction
#define BLACK_SBAR_FRAC_N 2 

// We take the 1/5 of the image regarding rows
#define BLACK_SBAR_FRAC_M 5

// Upper band to be removed in the input images
#define NOISE_BAND_FRAC 20

// The icons are located in the 1/6 left of the image
#define ICONS_BAND_FRAC 6 


/*
	Loads the image.
	If the image can't be loaded, exit the program.

	@param name Name of the image to load
	@return The loaded image
*/
Mat loadImage(const string &fileName) {
	Mat im = imread(fileName);
	if (im.data == NULL) {
		cerr << "Image not found: " << fileName << endl;
		exit(0);
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

/*
	Rotates the original image to make it straight. We look for the band up in
	the image to look for the longest line we use the canny filter to detect
	edges and hough transform to detect straight lines.

	@param im_gray		The original gray image
	@param im_rotated	The result of the rotation
	@return the degree of the rotation
*/
double rotation(const Mat& im_gray, Mat& im_rotated)
{
	int im_rows = im_gray.rows;
	int im_cols = im_gray.cols;

	// First we take the upper portion of the image where the black band is 
	// located
	int solid_bar_rows = im_rows / BLACK_SBAR_FRAC_M;
	int solid_bar_cols = im_cols / BLACK_SBAR_FRAC_N;
	int noise_zone_rows = im_rows / NOISE_BAND_FRAC;
	int noise_zone_cols = im_cols / NOISE_BAND_FRAC;
	// We take a reference on the zone containing the solid bar
	Mat im_bar_zone = im_gray(Range(noise_zone_rows, solid_bar_rows), Range(solid_bar_cols, im_cols - noise_zone_cols));
	Mat im_canny_edges;
	Canny(im_bar_zone, im_canny_edges, 50, 200, 3);
	vector<Vec4i> hough_lines;

	// The HoughLineP is called with an angular resolution of 0.1 degree
	HoughLinesP(im_canny_edges, hough_lines, 1, CV_PI / 1800, 50, 50, 10);
	int best_line_pos; //position of the longest line in the line's vector
	float max_norm = 0;
	float line_norm;
	for (int i = 0; i < hough_lines.size(); i++) {
		Vec4i l = hough_lines[i];
		line_norm = pow((l[2] - l[0]), 2) + pow((l[3] - l[1]), 2);
		if (line_norm > max_norm) {
			max_norm = line_norm;
			best_line_pos = i;
		}
	}

	// At the end of the loop we have the longest line
	// Calculating the angle between the line and a virtual straight horizontal 
	// line
	Vec4i best_line = hough_lines[best_line_pos];
	double rot_angle = atan((double)(best_line[3] - best_line[1]) / (double)(best_line[2] - best_line[0]));

	// Conversion on the angle from rad to degree
	rot_angle = rot_angle*(180 / CV_PI);

	//getting the rotation matrix with the image center as rotation axe
	Mat rot_mat = getRotationMatrix2D(Point2f(im_cols / 2, im_rows / 2), rot_angle, 1);

	im_bar_zone = 255;

	//rotation of the image
	warpAffine(im_gray, im_rotated, rot_mat, im_gray.size());

	return rot_angle;
}

/*
	Gets the correct rectangles that can be found on the given matrice.

	@param im_rgb	The given image
	@return A vector with all the founded rectangle
*/
vector<Rect> getRectangles(const Mat& im_rgb) {
	Mat im_gray;
	cvtColor(im_rgb, im_gray, COLOR_BGR2GRAY);
	int im_rows = im_gray.rows;
	int im_cols = im_gray.cols;

	std::cout << "rotated of : " << rotation(im_gray, im_gray) << " deg | ";
	
	// removing the the band on the top of scanned images
	// replacing the noised zone by white pixels
	int im_noise_band = im_rows / NOISE_BAND_FRAC;
	for (int i = 0; i < im_noise_band; i++) {
		for (int j = 0; j < im_cols; j++) {
			im_gray.at<uchar>(i, j) = (uchar)255; 
			im_gray.at<uchar>(im_rows - i - 1, j) = (uchar)255;
		}
	}

	//removing the side bands
	//replacing the noised zone by white pixels
	for (int j = 0; j < im_noise_band; j++) {
		for (int i = 0; i < im_rows; i++) {
			im_gray.at<uchar>(i, j) = (uchar)255; 
			im_gray.at<uchar>(i, im_cols - j - 1) = (uchar)255; 
		}
	}

	int icon_cols = im_cols / ICONS_BAND_FRAC;
	Mat subMat = im_gray(Range(0, im_rows - 1), Range(0, icon_cols - 1));
	subMat = 255;
	// Inverting the image this will be useful when detecting the lines of the 
	// grid 
	bitwise_not(im_gray, im_gray);
	// computing the sum of pixels values in the vertical and horizontal
	// directions
	Mat hist_v = Mat::zeros(im_cols, 1, CV_32SC1);
	Mat hist_h = Mat::zeros(im_rows, 1, CV_32SC1);
	int max_hist_h = 0;
	for (int i = 0; i < im_rows; i++) {
		// pointer to acces image's row this is faster than at template function
		uchar *im_row_ptr = im_gray.ptr<uchar>(i);

		for (int j = 0; j < im_cols; j++) {
			hist_h.at<int>(i) += im_row_ptr[j];
			hist_v.at<int>(j) += im_row_ptr[j];
		}

		// looking for the maximum value in the horizontal axe
		if (hist_h.at<int>(i) > max_hist_h) {
			max_hist_h = hist_h.at<int>(i);
		}
	}

	// looking for the maximum value in the accumulated values according to the
	// vertical axe
	int max_hist_v = 0;
	for (int j = 0; j < im_cols; j++) {
		if (hist_v.at<int>(j) > max_hist_v) {
			max_hist_v = hist_v.at<int>(j);
		}
	}

	// looking for the possibles horizontal and vertical lines in the form a line
	// is said to be probable if its corresponding pixel cumul is greather than 
	// the threshold calculating the thresholds
	float v_line_thresh = max_hist_v * LINE_THRESHOLD / 100.0;
	float h_line_thresh = max_hist_h * LINE_THRESHOLD / 100.0;
	int vline_count = 0;
	int hline_count = 0;
	int max_pos;
	vector<int> hlines;
	for (int i = 0; i < im_rows; i++) {

		if (hist_h.at<int>(i) > h_line_thresh) {
			// we now look for the pick value of this zone for it this the best 
			// place to be taken as line
			max_pos = i;
			int k = 0;
			do {
				if (hist_h.at<int>(i + ++k) > hist_h.at<int>(max_pos)) {
					max_pos = i + k;
				}
			} while (hist_h.at<int>(i + k) > h_line_thresh);

			hlines.push_back(max_pos); // pushing a new line
			i = i + k;                 // updating the loop counter
		}
	}

	// searching for probable lines in the vertical direction
	vector<int> vlines;
	for (int i = 0; i < im_cols; i++) {
		// looking for lines in the horizontal direction
		if (hist_v.at<int>(i) > v_line_thresh) {
			// we now look for the pick value of this zone for it this the best 
			// place to be taken as line
			max_pos = i;
			int k = 0;
			do {
				if (hist_v.at<int>(i + ++k) > hist_v.at<int>(max_pos)) {
					max_pos = i + k;
				}

			} while (hist_v.at<int>(i + k) > v_line_thresh);
			vlines.push_back(max_pos);  // pushing a new line
			i = i + k;                  // updating the loop counter
		}
	}

	// determining all the possible rectangles and removing impossible lines based
	// on the size of rectangles they form
	// the fields in the rectangles are as follows : p1.x|p1.y|p2.x|p2.y|diag

	// this table contains all the possibles rectangles
	Mat possibleRectangles = Mat((vlines.size() - 1) * (hlines.size() - 1), 5, CV_32S);

	int k = 0;
	int max_rect_diag = 0;
	for (int i = 0; i < hlines.size() - 1; i++) {
		for (int j = 0; j < vlines.size() - 1; ++k, ++j) {
			possibleRectangles.at<int>(k, 1) = vlines[j];
			possibleRectangles.at<int>(k, 2) = hlines[i];
			possibleRectangles.at<int>(k, 3) = vlines[j + 1];
			possibleRectangles.at<int>(k, 4) = hlines[i + 1];
			possibleRectangles.at<int>(k, 5) = pow((vlines[j + 1] - vlines[j]), 2) +
				pow((hlines[i + 1] - hlines[i]), 2);

			// looking for the maximum value at each iteration
			if (possibleRectangles.at<int>(k, 5) > max_rect_diag) {
				max_rect_diag = possibleRectangles.at<int>(k, 5);
			}
		}
	}

	// search for valid rectangles (the one having a size close to the maximum
	// size)
	float rect_thresh = (float)(max_rect_diag * RECT_THRESHOLD) / (float)100;
	Point p1, p2;
	vector<Rect> correctRectangles;
	for (int i = 0; i < possibleRectangles.rows; i++) {
		if (possibleRectangles.at<int>(i, 5) > rect_thresh) {
			p1.x = possibleRectangles.at<int>(i, 1) + COORD_OFFSET;
			p1.y = possibleRectangles.at<int>(i, 2) + COORD_OFFSET;

			p2.x = possibleRectangles.at<int>(i, 3) - COORD_OFFSET;
			p2.y = possibleRectangles.at<int>(i, 4) - COORD_OFFSET;

			correctRectangles.push_back(Rect(p1, p2));
		}
	}

	std::cout << correctRectangles.size() << " rectangles found !" << endl;

	return correctRectangles;
}

/*
	Slices a given matrice with the given rectangles.

	@param image	The given image
	@param rects	The vector containing the rectangles
	@return a vector with submatrice of the given matrice
*/
vector<Mat> slice(const Mat& image, const vector<Rect>& rects) {
	vector<Mat> images;
	for (const Rect& r : rects) {
		images.push_back(image.colRange(r.x, r.x + r.width)
			.rowRange(r.y, r.y + r.height));
	}
	return images;
}

/*
	Saves the thumbnails with the correct name and file descriptor.

	@param fileName			The name of the file
	@param subThumbnails	The matrices to save
*/
void saveSubThumbnails(const string& fileName, const vector<Mat>& subThumbnails, array<array<String, 2>, 7> iconLabels) {
	const string SAVE_DIR = "toast/";

	string scripter = fileName.substr(0, 3);
	string page = fileName.substr(3, 2);


	for (int i = 0; i < subThumbnails.size(); ++i) {
		// Creating the string
		string label = iconLabels[i / 5][0];
		string row = to_string(i / 5 + 1);
		string col = to_string((i % 5) + 1);
		string name = label + "_" + scripter + "_" + page + "_" + row + "_" + col;

		// Save the thumbnails
		imwrite(SAVE_DIR + name + ".png", subThumbnails.at(i));

		// Save the description file
		ofstream file(SAVE_DIR + name + ".txt");
		file << "#Groupe Adriano / Emmanuel / Guimel / Patrick\n";
		file << "label " + label + "\n";
		file << "form " << scripter << page << "\n";
		file << "scripter " << scripter << "\n";
		file << "page " << page << "\n";
		file << "row " << row << "\n";
		file << "col " << col << "\n";
		file << "size " + iconLabels[i / 5][1];
		file.close();
	}
}


const String TEMPLATE_PATH = "templates/";
const map<String, Mat> ICONS = {
	{ "accident", loadImage(TEMPLATE_PATH + "accident.png") },
	{ "bomb", loadImage(TEMPLATE_PATH + "bomb.png")},
	{ "car", loadImage(TEMPLATE_PATH + "car.png") },
	{ "casualty", loadImage(TEMPLATE_PATH + "casualty.png") },
	{ "electricity", loadImage(TEMPLATE_PATH + "electricity.png") },
	{ "fire", loadImage(TEMPLATE_PATH + "fire.png") },
	{ "fire_brigade", loadImage(TEMPLATE_PATH + "fire_brigade.png") },
	{ "flood", loadImage(TEMPLATE_PATH + "flood.png") },
	{ "gas", loadImage(TEMPLATE_PATH + "gas.png") },
	{ "injury", loadImage(TEMPLATE_PATH + "injury.png") },
	{ "paramedics", loadImage(TEMPLATE_PATH + "paramedics.png") },
	{ "person", loadImage(TEMPLATE_PATH + "person.png") },
	{ "police", loadImage(TEMPLATE_PATH + "police.png") },
	{ "roadblock", loadImage(TEMPLATE_PATH + "roadblock.png") }
};

const map<String, Mat> ICONS_TEXT = {
	{ "small", loadImage(TEMPLATE_PATH + "small.png")},
	{ "medium", loadImage(TEMPLATE_PATH + "medium.png") },
	{ "large", loadImage(TEMPLATE_PATH + "large.png") }
};

/*
	Classifies a cropped icon and returns a descriptive string.

	@param im		The image
	@param icons	The map of icons
	@return The descriptive string of the image
*/
String classifyCroppedIcon(const Mat& im, const map<String, Mat>& icons) {
	Mat result;
	double currentVal;
	double maxVal = -1;
	String res = "";
	for (auto i : icons) {
		matchTemplate(im, i.second, result, CV_TM_CCOEFF_NORMED);
		minMaxLoc(result, NULL, &currentVal);

		if (currentVal >= 0.5 && currentVal > maxVal) {
			maxVal = currentVal;
			res = i.first;
		}
	}

	return res;
}

/*
	Isolates all the left type images.

	@param image		The image
	@param rectangles	The rectangle where are the icons
	@param result		The result of the isolation
*/
void isolateAndClassifyIcons(const Mat& image, vector<Rect>& rectangles, array<array<String, 2>, 7>& result) {
	//TODO strategy for weird rectangle-quantities, general handling for page 22
	if (rectangles.size() != 35) {
		std::cout << "Skipping this image because of bad rectangle count!" << endl;
		return;
	}

	//get arithmetic mean of all lines
	int meanAccumY = 0;
	int meanAccumH = 0;
	int rectCount = 0;
	int lineCount = 0;
	for (const Rect& r : rectangles) {
		int arithMeanY = 0;
		int arithMeanH = 0;
		meanAccumY += r.y;
		meanAccumH += r.height;
		rectCount++;
		if (rectCount % 5 == 0) {
			arithMeanY = (meanAccumY / 5.0);
			arithMeanH = (meanAccumH / 5.0);

			// Setup a rectangle to define your region of interest
			// TODO fine-tuning the cropping (if not robust enough)
			cv::Rect myROI(rectangles[rectCount-5].x - (int)(image.cols*0.15), arithMeanY, (int)(image.cols*0.08), arithMeanH);
			//

			// Crop the full image to that image contained by the rectangle myROI
			// Note that this doesn't copy the data
			cv::Mat croppedRef(image, myROI);

			cv::Mat cropped;
			//TODO is it faster to not copy?
			// Copy the data into new matrix
			croppedRef.copyTo(cropped);

			//reset accumulators
			meanAccumY = 0;
			meanAccumH = 0;

			result[lineCount][0] = classifyCroppedIcon(cropped, ICONS);
			result[lineCount][1] = classifyCroppedIcon(cropped, ICONS_TEXT);
			if (result[lineCount][0].empty()) {
				//inspect wrong recognition
				imwrite("dump.png", cropped);
			}
			lineCount++;
		}
	}
}


int main(void) {
	clock_t start_time = clock();
	const string PATH_IMGDB = "donnees/";
	vector<string> unusedImages;

	int processed_images = 0;
	int successful_images = 0;
	//TODO further improvements through using multithreading

	for (string filename : getFilesName(PATH_IMGDB.c_str(), ".png")) {
		Mat im_rgb = imread(PATH_IMGDB + filename);
		if (im_rgb.data != NULL) {

			Mat hsv;
			Scalar hsv_l(20, 60, 60);
			Scalar hsv_h(130, 255, 255);
			cvtColor(im_rgb, hsv, CV_BGR2HSV);
			Mat bw;
			inRange(hsv, hsv_l, hsv_h, bw);

			bitwise_not(bw, bw);
			Mat test;

			Mat im_bw;
			cvtColor(im_rgb, im_bw, CV_BGR2GRAY);
		    
			bitwise_and(im_bw, bw, test);
			imwrite(filename + filename, test);
			bitwise_not(test, test, bw);

			imwrite(filename, test);




			processed_images++;
			std::cout << filename << " | ";
			vector<Rect> res = getRectangles(im_rgb);

			if (res.size() == 35) { // TODO what to do with the images with wrong rectangle counts
				array<array<String, 2>, 7> icons;

				isolateAndClassifyIcons(im_rgb, res, icons);
				//TODO make sure that we have no remnants of the black line by blocking everything
				// in the sub-images that is not blue
				saveSubThumbnails(filename, slice(im_rgb, res), icons);
				successful_images++;
			}
			else {
				unusedImages.push_back(filename);
				std::cout << "Warning: skipped image because of bad rectangle count" << endl;
			}
		}
	}

	clock_t end_time = clock();
	clock_t tot_time = (end_time - start_time);
	std::cout << "Total execution time:      " << tot_time / 1000 << "s" << endl;
	std::cout << "Total execution time:      " << tot_time / 1000 / 60 << "min ";
	std::cout << ((tot_time / 1000) % 60) << "s" << endl;

	std::cout << "Images proc. " << successful_images << " / " << processed_images
		<< " (" << ((double)successful_images / processed_images) * 100 << "%)\n";
	std::cout << "avg. proc. time per image: " << tot_time / processed_images << "ms" << endl;

	std::cout << "\nUnused images\n";
	std::cout << "=============\n" << endl;
	for (const string& s : unusedImages) {
		if (s.substr(3, 3).find("22")) {
			std::cout << s << endl;
		}
	}

	system("PAUSE");
	return 0;
}
