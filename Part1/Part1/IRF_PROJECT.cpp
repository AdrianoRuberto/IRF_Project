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
#define LINE_THRESHOLD 50

// threshold for the size of found rectangles
#define RECT_THRESHOLD 80

// offset to apply to lines coordinates to avoid taking the borders
#define COORD_OFFSET 5

/*
Loads the image.
If the image can't be loaded, exit the program
@param name Name of the image to load
@return The loaded image
*/
Mat loadImage(const string &name) {
	Mat im = imread(name);
	if (im.data == NULL) {
		cerr << "Image not found: " << name << endl;
		exit(0);
	}
	return im;
}


/*
	Gets the correct rectangles that can be found on the given matrice
	@param im_rgb	The given image
	@return A vector with all the founded rectangle
*/
vector<Rect> getRectangles(const Mat& im_rgb) {
	Mat im_gray;
	cvtColor(im_rgb, im_gray, COLOR_BGR2GRAY);
	int im_rows = im_gray.rows;
	int im_cols = im_gray.cols;
	// removing the the band on the top of scanned images
	int im_noise_band = im_rows / NOISE_BAND_FRAC;
	for(int i = 0; i < im_noise_band; i++)
	{
		for(int j = 0; j < im_cols; j++)
		{
			im_gray.at<uchar>(i, j) = (uchar)255; //replacing the noised zone by white pixels
			im_gray.at<uchar>(im_rows - i - 1, j) = (uchar)255; //replacing the noised zone by white pixels
		}
	}
	//removing the side bands
	for(int j = 0; j < im_noise_band; j++)
	{
		for(int i = 0; i <im_rows; i++)
		{
			im_gray.at<uchar>(i, j) = (uchar)255; //replacing the noised zone by white pixels
			im_gray.at<uchar>(i, im_cols - j - 1) = (uchar)255; //replacing the noised zone by white pixels
		}
	}

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

	cout << correctRectangles.size() << " rectangles found !" << endl;

	return correctRectangles;
}

/*
	Slices a given matrice with the given rectangles
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
	Saves the thumbnails with the correct name and file descriptor
	@param fileName			The name of the file
	@param subThumbnails	The matrices to save
*/
void saveSubThumbnails(const string& fileName, const vector<Mat>& subThumbnails, array<String, 7> iconLabels) {
	const string SAVE_DIR = "results/";

	string scripter = fileName.substr(0, 3);
	string page = fileName.substr(3, 2);


	for (int i = 0; i < subThumbnails.size(); ++i) {
		// Creating the string
		string label;
		if (i >= 35) {
			label = "failure_in_complete_page";
		}
		else {
			label = iconLabels[i / 5];
		}
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
	Classifies a cropped icon and returns a descriptive string
*/
String classifyCroppedIcon(const Mat& icon) {

	// go through list of icons and find the best match
	Mat result;
	double currentVal;
	double maxVal = -1;
	auto res = ICONS.end();

	for (auto it = ICONS.begin(); it != ICONS.end(); it++) {
		matchTemplate(icon, it->second, result, CV_TM_CCOEFF_NORMED);
		minMaxLoc(result, NULL, &currentVal);
		
		if (currentVal > maxVal) {
			maxVal = currentVal;
			res = it;
		}
	}
	
	//cout << "Probability of match: " << currentVal << endl;
	if (maxVal < 0.5) {
		cout << "error classifying, confidence to low" << endl;
		return "not_classified";
	}

	// go through list of icon_texts and find the best match
	maxVal = -1;
	auto resT = ICONS_TEXT.end();
	for (auto it = ICONS_TEXT.begin(); it != ICONS_TEXT.end(); it++) {
		matchTemplate(icon, it->second, result, CV_TM_CCOEFF_NORMED);
		minMaxLoc(result, NULL, &currentVal);

		if (currentVal > maxVal) {
			maxVal = currentVal;
			resT = it;
		}
	}


	//TODO seperate this into two return values (we need it separated)
	String returnvalue = "";
	if (maxVal > 0.5) {
		returnvalue = resT->first + "_";
	}

	//return String-descriptor of classified icon
	//cout << "match: " << res->first << ", probability of match: " << currentVal << endl;
	return returnvalue + res->first;
}




/*
	Isolates all the left type images.
*/
void isolateAndClassifyIcons(const Mat& image, vector<Rect>& rectangles, array<String, 7>& result) {
	for (int i = 0; i < result.size(); ++i) {
		result[i] = "failure";
	}

	//TODO strategy for weird rectangle-quantities, general handling for page 22
	if (rectangles.size() != 35) {
		cout << "Skipping this image because of bad rectangle count!" << endl;
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
		// cout << r.y << endl;
		rectCount++;
		if (rectCount % 5 == 0) {
			arithMeanY = (meanAccumY / 5.0);
			arithMeanH = (meanAccumH / 5.0);
			
			// Setup a rectangle to define your region of interest
			// TODO fine-tuning the cropping (if not robust enough)
			cv::Rect myROI((int)(image.cols*0.1), arithMeanY, (int)(image.cols*0.06), arithMeanH);

			// Crop the full image to that image contained by the rectangle myROI
			// Note that this doesn't copy the data
			cv::Mat croppedRef(image, myROI);

			cv::Mat cropped;
			//TODO is it faster to not copy?
			// Copy the data into new matrix
			croppedRef.copyTo(cropped);
			//imwrite("dump.png", cropped);
			//system("PAUSE");
			
			//reset accumulators
			meanAccumY = 0;
			meanAccumH = 0;

			result[lineCount] = classifyCroppedIcon(cropped);
			lineCount++;
		}
	}
}

int main(void) {
	clock_t start_time = clock();
	const string PATH_IMGDB = "imgdb/";
	
	int processed_images = 0;
	int successful_images = 0;
	//TODO further improvements through using multithreading
	for (int i = 0; i < 50; ++i) {
		stringstream filename;
		filename << setfill('0') << setw(5) << i << ".png";
		Mat im_rgb = imread(PATH_IMGDB + filename.str());
		if (im_rgb.data != NULL) {
			processed_images++;
			cout << filename.str() << " | ";
			vector<Rect> res = getRectangles(im_rgb);

			if (res.size() == 35) { // TODO what to do with the images with wrong rectangle counts
				array<String, 7> icons;
				isolateAndClassifyIcons(im_rgb, res, icons);
				//TODO make sure that we have no remnants of the black line by blocking everything
				// in the sub-images that is not blue
				saveSubThumbnails(filename.str(), slice(im_rgb, res), icons);
				successful_images++;
			}
			else {
				cout << "Warning: skipped image because of bad rectangle count" << endl;
			}
		}
	}

	clock_t end_time = clock();
	clock_t tot_time = (end_time - start_time);
	cout << "total execution time:      " << tot_time/1000 << "s" << endl;
	cout << "total execution time:      " << tot_time / 1000 / 60 << "min ";
	cout << ((tot_time / 1000) % 60) << "s" << endl;
	cout << "processed images:          " << processed_images << endl;
	cout << "successfully proc. img:    " << successful_images << endl;
	cout << "avg. proc. time per image: " << tot_time / processed_images << "ms" << endl;

	system("PAUSE");
	return 0;
}
