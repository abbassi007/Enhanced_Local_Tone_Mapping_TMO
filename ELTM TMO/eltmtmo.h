#pragma once
#include"ReadWriteFile.h"
#include <cstdio>
#include<cstddef>




extern void eltm_tmo(FILE* in, int w, int h, int count, int depth, MEMORY *image_buffer, double gamma,
	bool flt, bool bigendian);
