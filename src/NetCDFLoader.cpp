/*
* Copyright (C) 2016
* Computer Graphics Group, The Institute of Computer Graphics and Algorithms, TU Wien
* Written by Tobias Klein <tklein@cg.tuwien.ac.at>
* All rights reserved.
*/

#include "NetCDFLoader.h"

#include <netcdf.h>
#include <QDebug>

bool NetCDFLoader::readData(QString &path, std::vector<std::vector<Atom> > &animation, int *nrFrames, QProgressBar *progressBar)
{
	// load NetCDF (Network Common Data Form) data
	// see http://www.unidata.ucar.edu/software/netcdf/docs
	int status, ncid, ndims, nvars, ngatts, unlimdimid;

	float rh_array[1][1];

	status = nc_open(path.toStdString().c_str(), NC_NOWRITE, &ncid);
	status = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);

	size_t dim_length;
	char name_in[NC_MAX_NAME + 1];

	qInfo() << "----------------------------------------";
	qInfo() << "DIMENSIONS";
	for (int i = 0; i < ndims; i++) {
		status = nc_inq_dimname(ncid, i, name_in);
		status = nc_inq_dimlen(ncid, i, &dim_length);
		qInfo() << i << +": " << name_in << "[" << dim_length << "]";
	}
	qInfo() << "----------------------------------------";
	qInfo() << "VARIABLES:";
	int nDims;
	for (int i = 0; i < nvars; i++) {
		status = nc_inq_varname(ncid, i, name_in);
		status = nc_inq_varndims(ncid, i, &nDims);
		qInfo() << name_in;
		int *dimIds = new int[nDims];
		status = nc_inq_vardimid(ncid, i, dimIds);
		for (int j = 0; j < nDims; j++) {
			status = nc_inq_dimname(ncid, dimIds[j], name_in);
            qInfo() << "Dimension: " << name_in;
		}

	}
	qInfo() << "----------------------------------------";
	qInfo() << "ATTRIBUTES:";
	for (int i = 0; i < ngatts; i++) {
		status = nc_inq_attname(ncid, i, NC_GLOBAL, name_in);
		qInfo() << name_in;
	}

	const int FRAMES = 500; // 50000 total
	(*nrFrames) = FRAMES;
	const int ATOMS = 4684;
	const int SPATIAL = 3;
	if (progressBar) {
		progressBar->setMaximum(FRAMES + 10);
		progressBar->setValue(10);
	}

	float *rh_vals = new float[FRAMES*ATOMS*SPATIAL];

	static size_t start[] = { 0, 0, 0 };
	static size_t count[] = { FRAMES, ATOMS, SPATIAL };
	int coord_id;
	status = nc_inq_varid(ncid, "coordinates", &coord_id);
	status = nc_get_vara_float(ncid, coord_id, start, count, rh_vals);

	if (progressBar) {
		progressBar->setValue(FRAMES);
	}

    for (size_t i = 0; i < FRAMES; i++) {
		std::vector<Atom> frame;
        for (size_t j = 0; j < ATOMS; j++) {
			Atom atom;
            for (size_t k = 0; k < SPATIAL; k++) {
				atom.position[k] = rh_vals[i * ATOMS * SPATIAL + j * SPATIAL + k];
			}
			atom.color = glm::vec3(0.341f, 0.776f, 0.921f);
			atom.radius = 1.4f;
			frame.push_back(atom);
		}
		animation.push_back(frame);
		if (progressBar) {
			progressBar->setValue(10 + i);
		}
	}

	delete rh_vals;
	progressBar->setValue(0);
	return true;
}
