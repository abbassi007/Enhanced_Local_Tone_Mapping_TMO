#pragma once
#include"ReadWriteFile.h"
#include <cstdio>
#include<cstddef>


struct ELTM_Params
{
	double epsilon = 0.000001; 
	int rS = 3; 
	double epsilonS = 0.1;
	double epsilonL = 0.1;
	double lamdaF = 0.02;
	double lamdaC = 1; 
	int tauR = 5; 
	double etaF = 1; 
	double etaC = 1.5; 
	double p = 1;
	double k = 1;
	double m = -2.7; 
	double BPCmax = 0.9;
	double BPCmin = 0.1;
	double s = 1;
};

extern void eltm_tmo(FILE* in, int w, int h, int count, int depth, MEMORY *image_buffer, double gamma,
	bool flt, bool bigendian, const ELTM_Params& params);
