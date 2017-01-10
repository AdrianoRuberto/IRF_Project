#include "stdafx.h"
#include "ARFFManager.h"

ofstream& ARFFManager::addAttribute(const Attribute & att, ofstream& f)
{
	attributes.push_back(att);
	f << "@ATTRIBUTE " << att.name << " " << att.type << endl;
	return f;
}

ofstream& ARFFManager::addDatas(const string & line, ofstream& f)
{
	static bool first = true;

	if (first) {
		f << "@DATA" << endl;
		first = false;
	}

	datas.push_back(line);
	f << line << endl;
	return f;
}

ARFFManager::ARFFManager(const string& s, ostream& f)
{
	f << "@RELATION " << s << endl;

}


ARFFManager::~ARFFManager()
{
}
