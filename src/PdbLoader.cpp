/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#include "PdbLoader.h"

#include <algorithm>
#include <QFile>
#include <QDebug>
#include <QTextStream>

// Color scheme taken from http://life.nthu.edu.tw/~fmhsu/rasframe/COLORS.HTM
const glm::vec3 residueColors[] =
{
	glm::vec3(200,200,200) / 255.0f,     // ALA      dark grey
	glm::vec3(20,90,255) / 255.0f,       // ARG      blue       
	glm::vec3(0,220,220) / 255.0f,       // ASN      cyan   
	glm::vec3(230,10,10) / 255.0f,       // ASP      bright red
	glm::vec3(255,200,50) / 255.0f,      // CYS      yellow 
	glm::vec3(0,220,220) / 255.0f,       // GLN      cyan   
	glm::vec3(230,10,10) / 255.0f,       // GLU      bright red
	glm::vec3(235,235,235) / 255.0f,     // GLY      light grey
	glm::vec3(130,130,210) / 255.0f,     // HID      pale blue
	glm::vec3(130,130,210) / 255.0f,     // HIE      pale blue
	glm::vec3(130,130,210) / 255.0f,     // HIP      pale blue
	glm::vec3(130,130,210) / 255.0f,     // HIS      pale blue
	glm::vec3(15,130,15) / 255.0f,       // ILE      green  
	glm::vec3(15,130,15) / 255.0f,       // LEU      green  
	glm::vec3(20,90,255) / 255.0f,       // LYS      blue       
	glm::vec3(255,200,50) / 255.0f,      // MET      yellow 
	glm::vec3(50,50,170) / 255.0f,       // PHE      mid blue
	glm::vec3(220,150,130) / 255.0f,     // PRO      flesh  
	glm::vec3(250,150,0) / 255.0f,       // SER      orange 
	glm::vec3(250,150,0) / 255.0f,       // THR      orange 
	glm::vec3(180,90,180) / 255.0f,      // TRP      pink   
	glm::vec3(50,50,170) / 255.0f,       // TYR      mid blue
	glm::vec3(15,130,15) / 255.0f        // VAL      green  
};

const glm::vec3 AtomColors[] =
{
	glm::vec3(200,200,200) / 255.0f,	// C        light grey
	glm::vec3(255,255,255) / 255.0f,	// H        white       
	glm::vec3(143,143,255) / 255.0f,	// N        light blue
	glm::vec3(240,0,0) / 255.0f,		// O        red         
	glm::vec3(255,165,0) / 255.0f,		// P        orange      
	glm::vec3(255,200,50) / 255.0f,		// S        yellow    
	glm::vec3(255, 0,255) / 255.0f		// A        purple   
};

// RCSB Protein Data Bank File Format
// http://deposit.rcsb.org/adit/docs/pdb_atom_format.html#ATOM
bool PdbLoader::readAtomData(QString &path, std::vector<Atom> &atoms)
{

	QFile file(path);
		
	if (!file.exists()) {
        qCritical() << "Error loading file: " << path;
		return false;
	}

	if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Error loading file: " << file.errorString();
	}

	QString pdbName = file.fileName();
	QList<QChar> chains;
	QTextStream in(&file);

	while (!in.atEnd()) {
		QString line = in.readLine();
		QStringList fields = line.split(",");

        if (line.startsWith("ATOM") || (pdbName.contains("3irl") && line.startsWith("HETATM"))) {

			QStringRef xString(&line, 30, 8);
			float x = xString.toFloat();
			QStringRef yString(&line, 38, 8);
			float y = yString.toFloat();
			QStringRef zString(&line, 46, 8);
			float z = zString.toFloat();

			QStringRef name(&line, 12, 4);
			name = name.trimmed();
			
			QStringRef symbol(&line, 76, 2);
			symbol = symbol.trimmed();

			//if (std::find(AtomHelper::atomSymbols.begin(), AtomHelper::atomSymbols.end(), x) != AtomHelper::atomSymbols.end()) {
				//var t = Regex.Replace(name, @"[\d - ]", string.Empty).Trim();
				//symbol = t[0].ToString();
			//}

			auto it_atomSymbols = std::find(AtomHelper::atomSymbols.begin(), AtomHelper::atomSymbols.end(), symbol);
			auto symbolId = std::distance(AtomHelper::atomSymbols.begin(), it_atomSymbols);
            if (it_atomSymbols == AtomHelper::atomSymbols.end()) {
                qDebug() << pdbName << ": Atom symbol not available at line : " << line;
				symbolId = 6;
			}
			

			// Skip hydrogen atoms 
			if (symbolId == 1) continue;

			float radius = AtomHelper::atomRadii[symbolId];

			QStringRef residueName(&line, 17, 3);
			//residueName = residueName.trimmed();

			auto it_residueNames = std::find(AtomHelper::residueNames.begin(), AtomHelper::residueNames.end(), residueName);
			auto residueId = std::distance(AtomHelper::residueNames.begin(), it_residueNames);
            if (residueId < 0) {
                qDebug() << pdbName << ": Residue symbol not available at line: " + line;
			}

			if (residueName == "HOH") continue;

			QStringRef residueIndexStr(&line, 22, 4);
			int residueIndex = residueIndexStr.toInt();
			

			QCharRef chain = line[21];
			if (chains.contains(chain)) {
				chains.append(chain);
			}
			int chainId = chains.indexOf(chain);

			Atom atom;
			atom.radius = radius;
			atom.color = AtomColors[symbolId];
			atom.name = name.toString();
			atom.symbol = symbol.toString();
			atom.symbolId = symbolId;
			atom.residueName = residueName.toString();
			atom.residueId = residueId;
			atom.residueIndex = residueIndex;
			atom.chain = chain;
			atom.chainId = chainId;
			atom.position = glm::vec3(-x,y,z);

			atoms.push_back(atom);
		}

	}
	file.close();

	return true;
}

void PdbLoader::offsetAtoms(std::vector<Atom> &atoms, glm::vec3 offset)
{
    for (auto i = 0; i < atoms.size(); i++) {
		atoms[i].position -= offset;
	}
}

void PdbLoader::computeBounds(std::vector<Atom> &atoms, glm::vec3 &bbSize, glm::vec3 &bbCenter)
{
	auto bbMin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	auto bbMax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (size_t i = 0; i < atoms.size(); i++) {
		auto atom = atoms[i];
		bbMin = glm::min(bbMin, atom.position);
		bbMax = glm::max(bbMax, atom.position);
	}

	bbSize = bbMax - bbMin;
	bbCenter = bbMin + bbSize * 0.5f;
}

void PdbLoader::centerAtoms(std::vector<Atom> &atoms)
{
	glm::vec3 bbSize;
	glm::vec3 bbCenter;

	computeBounds(atoms, bbSize, bbCenter);

	offsetAtoms(atoms, bbCenter);
}


Atom::Atom() 
{
}

Atom::~Atom() 
{
}

const std::vector<float> AtomHelper::atomRadii = { 1.548f, 1.100f, 1.400f, 1.348f, 1.880f, 1.808f, 1.5f };
const std::vector<QString> AtomHelper::atomSymbols = { "C", "H", "N", "O", "P", "S", "A" };
const std::vector<QString> AtomHelper::residueNames = { "ALA", "ARG", "ASN", "ASP", "CYS", "GLN", "GLU", "GLY", "HID", "HIE", "HIP", "HIS", "ILE", "LEU", "LYS", "MET", "PHE", "PRO", "SER", "THR", "TRP", "TYR", "VAL" };

