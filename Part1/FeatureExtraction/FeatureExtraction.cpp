// FeatureExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ARFFManager.h"

using namespace std;

int main()
{
	ARFFManager manager;
	manager.addAttribute({ "LCC", "string" });
	manager.addAttribute({ "sepallength", "NUMERIC" });

	ofstream f("test.arff");

	manager.write(f);

	system("pause");
    return 0;
}

