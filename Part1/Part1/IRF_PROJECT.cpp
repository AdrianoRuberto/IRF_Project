//getting started with the open cv library
#include"stdafx.h"
#include "histogram.h"
#include "cv.h"
#include "highgui.h"
#include<iostream>
#include<fstream>
#include<math.h>
using namespace cv;
using namespace std;

int main(void) {

	//declaration section
	string imName = "00000.png";
	Mat im_rgb, im_gray;
	int im_rows, im_cols, im_noise_band, i, j,k, vline_count, hline_count;
	int max_hist_h, max_hist_v, max_rect_diag, rect_count;
	float v_line_thresh, h_line_thresh, rect_thresh;
	std::vector<int> vlines, hlines;//
	cv::vector<Point2i> rect_p1, rect_p2;//coordinates for the rectangles to be taken in the form
	int max_pos;
	uchar* im_row_ptr;//pointer to acces image's row this is faster than at template function
	Point p1, p2, pxy;//point to plot lines on the image
	//somes constants that are to used in the code
	const int noise_band_frac = 20; // upper band to be removed in the input images
	const int line_threshold = 50;//threshold for selecting line in the horizontal and vertical directions
	const int rect_threshold = 80;//threshold for the size of found rectangles
	const int coord_offset = 10;// offset to apply to lines coordinates to avoid taking the borders

	//----------------------------------------
	im_rgb = imread(imName); //reading the input image
	if (im_rgb.data == NULL) {
		cerr << "Image not found " << imName << endl;
		exit(0);
	}
	cvtColor(im_rgb, im_gray, COLOR_BGR2GRAY);
	im_rows = im_gray.rows;
	im_cols = im_gray.cols;
	//removing the the band on the top of scanned images
	//this band is located approximatively at the 1/35 of the top
	im_noise_band = im_rows / noise_band_frac + 20;
	for (i = 0; i < im_noise_band; i++)
	{
		for (j = 0; j < im_cols; j++)
		{
		   im_gray.at<uchar>(i, j) = (uchar)255; //replacing the noised zone by white pixels
		}
	}
	im_gray = 255 - im_gray;/*inverting the image this will be useful when detecting the lines of the grid*/
		
							
	/*computing the summ of pixels values in the vertical and horizontal directions*/
	Mat hist_v = Mat::zeros(im_cols,1,CV_32SC1);/* initialization of the accumulators*/
	Mat hist_h = Mat::zeros(im_rows, 1, CV_32SC1);
	max_hist_h = 0;
	//opening a file for debuging
	ofstream outfileh("h_hist.txt");
	ofstream outfilev("v_hist.txt");
	for (i = 0; i < im_rows; i++)
	{
		im_row_ptr = im_gray.ptr<uchar>(i);//taking the pointer on the image row
		for (j = 0; j < im_cols; j++)
		{
			hist_h.at<int>(i) += im_row_ptr[j];//im_gray.at<uchar>(i,j) ; //accumulating the pixel in the whole considered row
			hist_v.at<int>(j) += im_row_ptr[j];//im_gray.at<uchar>(i,j); //progressive accumulation of pixel values in the vertical direction
		}
		if (hist_h.at<int>(i) > max_hist_h)//looking for the maximum value in the horizontal axe
		{
			max_hist_h = hist_h.at<int>(i);
		}
	}
//outputing the values for debuging
	//outputing the h_count
	//outfile << "h_hist = {";
	for (i = 0; i < im_rows; i++)
	{
		outfileh << hist_h.at<int>(i) <<endl;
	}
	//outfile << "};" << endl << endl;
	//outputing the v_count
	//outfile << "v_hist = {";
	for (j = 0; j < im_cols; j++)
	{
		outfilev << hist_v.at<int>(j) <<endl;
	}
	//outfile << "};" << endl << endl;
  //looking for the maximum value in the accumulated values according to the vertical axe
	max_hist_v = 0;
	for (j = 0; j < im_cols; j++)
	{
		//cout << "vHist("<<j<<") = " << hist_v.at<int>(j) << endl;
		if (hist_v.at<int>(j) > max_hist_v)
		{
			max_hist_v = hist_v.at<int>(j);
		}
     }

	//looking for the possibles horizontal and vertical lines in the form a line is said to be probable if 
	//its correspondint pixel cumul is greather than the threshold
	//calculating the thresholds
	v_line_thresh = ((float)max_hist_v*line_threshold) / (float)100;
	h_line_thresh = ((float)max_hist_h*line_threshold)/ (float)100;
	vline_count = hline_count = 0;

	/*searching for probable lines in the horizontal direction*/
	for (i = 0; i < im_rows; i++)
	{
		/*looking for lines in the horizontal direction*/
		if (hist_h.at<int>(i) > h_line_thresh)
		{/*we now look for the pick value of this zone for it this the best place to be taken as line*/
			max_pos = i;
			k = 0;
			do
			{
				if (hist_h.at<int>(i + ++k) > hist_h.at<int>(max_pos))
				{
					max_pos = i + k;
				}
				
			} while (hist_h.at<int>(i+k) > h_line_thresh);
			hlines.push_back(max_pos);/*pushing a new line*/
			i = i + k; /*updating the loop counter*/
		}
	}

	/*searching for probable lines in the vertical direction*/
	for (j = 0; j < im_cols; j++)
	{
		//looking for lines in the horizontal direction
		if (hist_v.at<int>(j) > v_line_thresh)
		{//we now look for the pick value of this zone for it this the best place to be taken as line
			max_pos = j;
			k = 0;
			do
			{
				if (hist_v.at<int>(j + ++k) > hist_v.at<int>(max_pos))
				{
					max_pos = j + k;
				}

			} while (hist_v.at<int>(j + k) > v_line_thresh);
			vlines.push_back(max_pos);//pushing a new line
			j = j + k; //updating the loop counter
		}
	}
// determining all the possible rectangles and removing impossible lines based on the size of rectangles they form
	//the fields in the rectangles are as follows : p1.x|p1.y|p2.x|p2.y|diag	
	Mat rectangles = Mat((vlines.size() - 1)*(hlines.size() - 1),5,CV_32S);/*this table contains all the possibles rectangles*/
	k = 0;
	max_rect_diag = 0;
	for (i=0; i<hlines.size() - 1;i++)
	{
		for (j=0; j<vlines.size()-1;j++)
		{
			rectangles.at<int>(k,1) = vlines[j];
			rectangles.at<int>(k,2) = hlines[i];
			rectangles.at<int>(k,3) = vlines[j+1];
			rectangles.at<int>(k,4) = hlines[i+1];
			rectangles.at<int>(k,5) = pow((vlines[j + 1]- vlines[j]),2) + pow((hlines[i + 1]- hlines[i]),2);
			/*looking for the maximum value at each iteration*/
			if (rectangles.at<int>(k,5)>max_rect_diag)
			{
				max_rect_diag = rectangles.at<int>(k,5);
			}
			k++;
		}
	}
	rect_thresh = (float)(max_rect_diag*rect_threshold) / (float)100;
	/*search for valid rectangles (the one having a size close to the maximum size)*/
	for (k = 0; k <rectangles.rows; k++)
	{
		if (rectangles.at<int>(k, 5) > rect_thresh)
		{
			p1.x = rectangles.at<int>(k,1) + coord_offset; 
			p1.y = rectangles.at<int>(k,2) + coord_offset;
			p2.x = rectangles.at<int>(k,3) - coord_offset;
			p2.y = rectangles.at<int>(k,4) - coord_offset;
			rect_p1.push_back(p1);
			rect_p2.push_back(p2);
		}
	}
	cout << rect_p1.size() << " Objects found !" << endl;
	/*drawing lines rectangles for verification purposes*/
	/*horizontal lines*/
	for (i = 0; i < rect_p1.size(); i++)
	{   /*horizontal lines*/
		p1= rect_p1[i];
		p2.x = rect_p2[i].x;
		p2.y = rect_p1[i].y;
		line(im_rgb, p1, p2, Scalar(0, 0, 255),4); /*a red line is drawned*/
		
		p2 = rect_p2[i];
		p1.x = rect_p1[i].x;
		p1.y = rect_p2[i].y;
		line(im_rgb, p1, p2, Scalar(0, 0, 255), 4); /*a red line is drawned*/
		/*vertical lines*/
		p1 = rect_p1[i];
		p2.x = rect_p1[i].x;
		p2.y = rect_p2[i].y;
		line(im_rgb, p1, p2, Scalar(0, 0, 255), 4); /*a red line is drawned*/

		p2 = rect_p2[i];
		p1.x = rect_p2[i].x;
		p1.y = rect_p1[i].y;
		line(im_rgb, p1, p2, Scalar(0, 0, 255), 4); /*a red line is drawned*/

		
	}

	cv::imwrite("im_result.png", im_rgb);
	int reduction = 2;
	Size tailleReduite(im_cols / reduction, im_rows / reduction);
	Mat imreduite = Mat(tailleReduite, CV_8UC3); 
	cv::resize(im_rgb, imreduite, tailleReduite);
	cv::namedWindow("reduced image", WINDOW_NORMAL);
	cv::imshow("reduced image", im_rgb);

	
	cv::waitKey(0);
	return 0;
}
