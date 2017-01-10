#pragma once


#include <vector>
#include <fstream>
#include <string>
using namespace std;

struct Attribute {
	string name;
	string type;
};

class ARFFManager
{
private:
	vector<Attribute> attributes;
	vector<string> datas;
public:

	ofstream& addAttribute(const Attribute& att, ofstream& f);
	ofstream& addDatas(const string& line, ofstream& f);

	ARFFManager(const string& s, ostream& f);
	~ARFFManager();
};

