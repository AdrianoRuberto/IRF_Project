// FeatureExtraction.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ARFFManager.h"
#include <string>
#include "dirent.h"
#include <iostream>
using namespace std;


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

int main()
{
	
	for (string s : getFilesName("..//Part1//Results", ".png")) {
		cout << s << endl;
	}

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

