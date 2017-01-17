#pragma once


#include <vector>
#include <fstream>
#include <string>
using namespace std;

struct Attribute {
	string name;
	string type;

	bool operator==(const Attribute& other) {
		return other.name == name && other.type == type;
	}
};

class ARFFManager
{
private:
	string relation;
	vector<Attribute> attributes;
	vector<string> datas;
public:

    void addAttribute(const Attribute& att);
	void addDatas(const string& line);

	void write(ofstream& f);

	ARFFManager(const string& relation);
	~ARFFManager();
};

