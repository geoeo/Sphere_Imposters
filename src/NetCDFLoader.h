/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#pragma once

#include <vector>
#include <QProgressBar>

#include "Commons.h"

class NetCDFLoader
{
	public:

        static bool readData(QString &path, std::vector<std::vector<Atom> > &animation, int *nrFrames, QProgressBar *progressBar = nullptr);
};
