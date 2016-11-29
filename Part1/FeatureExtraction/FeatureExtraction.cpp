// FeatureExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ARFFManager.h"
#include <string>
using namespace std;

int main()
{
	ARFFManager manager("IRF_Project");
	manager.addAttribute({ "LCC", "string" });
	manager.addAttribute({ "sepallength", "NUMERIC" });
	manager.addAttribute({ "class", "{Accident, Bomb, Car, Casualty, Electricity, Fire, Fire brigade, Flood, Gas, Injury, Paramedics, Person, Police, Road block" });
	
	manager.addDatas("");

	ofstream f("test.arff");

	manager.write(f);

	system("pause");
    return 0;
}

