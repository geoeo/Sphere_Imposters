/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <QString>

#include "Vector.h"
#include "Commons.h"

class AtomHelper
{
public:
	const static std::vector<float> atomRadii;
	const static std::vector<QString> atomSymbols;
	const static std::vector<QString> residueNames;
	const static glm::vec3 residueColors[];
	const static glm::vec3 AtomColors[];
};

class PdbLoader
{
public:
	static bool readAtomData(QString &path, std::vector<Atom> &atoms);

	static void centerAtoms(std::vector<Atom> &atoms);

	static void offsetAtoms(std::vector<Atom> &atoms, glm::vec3 offset);

	static void computeBounds(std::vector<Atom> &atoms, glm::vec3 &bbSize, glm::vec3 &bbCenter);

};
