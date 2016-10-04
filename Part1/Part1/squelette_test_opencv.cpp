//////////////////////////////////////////////////////////////////////////
// Module IRF, 5-iNFO
// Projet, séance 1
// thème : premier pas en OpenCV
// contenu : charge, affiche, réduction, calcul et affichage d'histogramme
// auteur : Luce Morin
// date : 4/10/2010
//////////////////////////////////////////////////////////////////////////

#include "histogram.h"

#include "cv.h"
#include "highgui.h"
using namespace cv;

#include <iostream>
using namespace std;


int main (void) {

	//charge et affiche l'image 
	string imName = "00000.png";
	Mat im = imread(imName);
	if(im.data == NULL){
		cerr << "Image not found: "<< imName << endl;
		exit(0);
	}
	imshow("exemple1", im);

	//applique une reduction de taille d'un facteur 5
	//ici modifier pour ne reduire qu'a l'affichage 
	//comme demande dans l'enonce
	int reduction = 5;
	Size tailleReduite(im.cols/reduction, im.rows/reduction);
	Mat imreduite = Mat(tailleReduite,CV_8UC3); //cree une image à 3 canaux de profondeur 8 bits chacuns
	resize(im,imreduite,tailleReduite);
	imshow("image reduite", imreduite);

	computeHistogram("histogramme", im);

	//termine le programme lorsqu'une touche est frappee
	waitKey(0);
}
