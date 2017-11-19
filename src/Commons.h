/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#pragma once

#include <glm/glm.hpp>
#include <QString>

class Atom
{
public:

	Atom();
	~Atom();

	float radius;

    int chainId; // peptide chain this atom belongs to
    int symbolId;
    int residueId; // residue (functional group) this atom belongs to
    int residueIndex;

    // protein secondary structure (alpha helices and beta sheets)
	QString helixName = "";
	int helixIndex = -1;

	QString sheetName = "";
	int sheetIndex = -1;

	int nbHelicesPerChain = -1;
	int nbSheetsPerChain = -1;

	QString name;
	QString chain;
	QString symbol;
	QString residueName;

	glm::vec3 position;
	glm::vec3 color;
};
