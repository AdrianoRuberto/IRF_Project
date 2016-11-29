#include "stdafx.h"
#include "ARFFManager.h"


void ARFFManager::write(ofstream& file)
{
	for (const Attribute& a : attributes) {
		file << "@ATTRIBUTE " << a.name << " " << a.type << endl;
	}
}

void ARFFManager::addAttribute(const Attribute & att)
{
	attributes.push_back(att);
}

ARFFManager::ARFFManager()
{
}


ARFFManager::~ARFFManager()
{
}
