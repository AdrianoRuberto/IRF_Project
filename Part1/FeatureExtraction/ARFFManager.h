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
	string relation;
	vector<Attribute> attributes;
	vector<string> datas;
public:

	void write(ofstream& f);
	void addAttribute(const Attribute& att);
	void addDatas(const string& line);

	ARFFManager(const string& s);
	~ARFFManager();
};

