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
public:

	void write(ofstream& f);
	void addAttribute(const Attribute& att);
	ARFFManager();
	~ARFFManager();
};

