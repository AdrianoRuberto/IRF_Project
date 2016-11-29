#include "stdafx.h"
#include "ARFFManager.h"


void ARFFManager::write(ofstream& file)
{
	file << "@RELATION " << relation << endl;

	for (const Attribute& a : attributes) {
		file << "@ATTRIBUTE " << a.name << " " << a.type << endl;
	}

	file << "@DATA" << endl;
	for (const string& s : datas) {
		file << s << endl;
	}
}

void ARFFManager::addAttribute(const Attribute & att)
{
	attributes.push_back(att);
}

void ARFFManager::addDatas(const string & line)
{
	datas.push_back(line);
}

ARFFManager::ARFFManager(const string& s)
{
	relation = s;
}


ARFFManager::~ARFFManager()
{
}
