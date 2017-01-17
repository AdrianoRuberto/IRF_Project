#include "stdafx.h"
#include "ARFFManager.h"

void ARFFManager::addAttribute(const Attribute & att)
{
	if (find(attributes.begin(), attributes.end(), att) == attributes.end()) {
		attributes.push_back(att);
	}
}

void ARFFManager::addDatas(const string & line)
{
	datas.push_back(line);
}

void ARFFManager::write(ofstream & f)
{
	f << "@RELATION " << relation << endl;

	for (Attribute att : attributes) {
		f << "@ATTRIBUTE " << att.name << " " << att.type << endl;
	}

	f << "@DATA" << endl;
	for (string data : datas) {
		f << data << endl;
	}
}

ARFFManager::ARFFManager(const string& s)
{
	relation = s;

}


ARFFManager::~ARFFManager()
{
}
