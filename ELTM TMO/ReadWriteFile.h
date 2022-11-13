#pragma once

#pragma warning(disable : 4996)

#ifdef _WIN32
#include <corecrt_wstdio.h>
#endif
#include<stdio.h>
#include <cstdint>
#include"type.h"



FILE* open_PNM(const char* input_file, int& width, int& height, int& depth, int& precission, bool& pfm, bool& big_edian);
UWORD inline double_to_half(double v);
double  half_to_double(UWORD h);
bool Read_RGB_Comp(FILE* in, double& rf, double& gf, double& bf, double& y, int count, int depth, bool flt, bool bigendian, bool xyz);
bool Read_Y_Comp(FILE* in, double& y, int count, int depth, bool flt, bool bigendian, bool xyz);
double inline read_float(FILE* in, bool bigendian);

void write_ppm_file(FILE* fp, int width, int height, unsigned char* img);