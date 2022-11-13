#include"ReadWriteFile.h"

#include <math.h>


FILE* open_PNM(const char* input_file, int& width, int& height, int& depth, int& precision, bool& pfm, bool& big_endian)
{
	FILE* file = fopen(input_file, "rb");
	char id, type;
	int max_value;
	int parameter;
	if (file)
	{
		pfm = false;
		big_endian = false;

		if (fscanf(file, "%c%c\n", &id, &type) == 2)
		{
			if (id == 'P' && (type == '5' || type == '6' || type == 'f' || type == 'F'))
			{
				if (type == 'f' || type == 'F')
				{
					pfm = true;
				}
				if (type == '5' || type == 'f')
				{
					depth = 1;
				}
				else
				{
					depth = 3;
				}
				while ((id = getc(file)) == '#')
				{
					char buffer[256];
					fgets(buffer, sizeof(buffer), file);
				}
				ungetc(id, file);
				int parms;
				if (pfm)
				{
					double scale = 1.0;
					parms = fscanf(file, "%d %d %lg%*c", &width, &height, &scale);
					if (parms == 3)
					{
						if (scale < 0.0)
						{
							// is little-endian
							big_endian = false;
						}
						else
						{
							big_endian = true;
						}
						precision = 16;
					}
				}
				else
				{
					parms = fscanf(file, "%d %d %d%*c", &width, &height, &max_value);
					if (parms == 3)
					{
						precision = 0;
						while ((1 << precision) < max_value)
							precision++;
					}
				}

				if (parms == 3)
				{
					return file;
				}
				fclose(file);
				//print_error("Invalid PPM/PFM input", 19);
			}
			else
			{
				fclose(file);
				//print_error("Invalid PPM/PFM input", 19);
			}
		}
		else
		{
			fclose(file);
			//print_error("Invalid input file format", 19);
		}
		fclose(file);
	}
	else
	{
		//print_error("Cannot open PPM/PFM file", 19);
	}
	return nullptr;
}



bool Read_Y_Comp(FILE* in, double& y, int count, int depth,
	bool flt, bool bigendian, bool xyz)
{
	bool warn = false;
	// Read the HDR image parameters.
	if (depth == 3)
	{
		if (flt)
		{
			double rf, gf, bf;
			if (xyz)
			{
				double xf, yf, zf;
				// Convert from XYZ to RGB (the same colorspace as the LDR)
				xf = read_float(in, bigendian);
				yf = read_float(in, bigendian);
				zf = read_float(in, bigendian);
				if (xf < 0.0) xf = 0.0, warn = true;
				if (yf < 0.0) yf = 0.0, warn = true;
				if (zf < 0.0) zf = 0.0, warn = true;
				//
				if (isnan(zf))
				{
					//print_error("Error when reading source file", 21);
				}
				//
				// Convert from XYZ to RGB (the same colorspace as the LDR)
				rf = xf * 3.2404542 + yf * -1.5371385 + zf * -0.4985314;
				gf = xf * -0.9692660 + yf * 1.8760108 + zf * 0.0415560;
				bf = xf * 0.0556434 + yf * -0.2040259 + zf * 1.0570000;
			}
			else
			{
				rf = read_float(in, bigendian);
				gf = read_float(in, bigendian);
				bf = read_float(in, bigendian);
				//
				if (rf < 0.0) rf = 0.0, warn = true;
				if (gf < 0.0) gf = 0.0, warn = true;
				if (bf < 0.0) bf = 0.0, warn = true;
				if (isnan(bf))
				{
					//print_error("Error when reading source file", 21);
				}
			}
			y = (0.2126 * rf + 0.7152 * gf + 0.0722 * bf);
		}
		else
		{
			int r, g, b;
			int max = (1l << count) - 1;
			// Integer samples, three components
			if (count <= 8)
			{
				r = getc(in);
				g = getc(in);
				b = getc(in);
			}
			else
			{
				r = getc(in) << 8;
				r |= getc(in);
				g = getc(in) << 8;
				g |= getc(in);
				b = getc(in) << 8;
				b |= getc(in);
			}
			if (b < 0)
			{
				//print_error("Error when reading source file", 21);
			}
			y = (0.2126 * r + 0.7152 * g + 0.0722 * b) / max;
			if (xyz)
			{
				double rf, gf, bf;
				double xf = r, yf = g, zf = b;
				// Convert from XYZ to RGB (the same colorspace as the LDR)
				rf = xf * 3.2404542 + yf * -1.5371385 + zf * -0.4985314;
				gf = xf * -0.9692660 + yf * 1.8760108 + zf * 0.0415560;
				bf = xf * 0.0556434 + yf * -0.2040259 + zf * 1.0570000;
				r = int(rf), g = int(gf), b = int(bf);
				if (r < 0.0) r = 0, warn = true;
				if (g < 0.0) g = 0, warn = true;
				if (b < 0.0) b = 0, warn = true;
				if (r > max) r = max, warn = true;
				if (g > max) g = max, warn = true;
				if (b > max) b = max, warn = true;
				y = (0.2126 * rf + 0.7152 * gf + 0.0722 * bf) / max;
			}
		}
	}
	else
	{
		int g;
		if (flt)
		{
			double gf;
			gf = read_float(in, bigendian);
			if (gf < 0.0) gf = 0.0, warn = true;
			g = double_to_half(gf);
			y = gf;
		}
		else
		{
			if (count <= 8)
			{
				g = getc(in);
			}
			else
			{
				g = getc(in) << 8;
				g |= getc(in);
			}
			y = double(g) / ((1L << count) - 1);
		}
	}

	return warn;
}

bool Read_RGB_Comp(FILE* in, double& rf, double& gf, double& bf, double& y, int count, int depth,
	bool flt, bool bigendian, bool xyz)
{
	bool warn = false;

	// Read the HDR image parameters.
	if (depth == 3)
	{
		if (flt)
		{
			if (xyz)
			{
				double xf, yf, zf;
				// Convert from XYZ to RGB (the same colorspace as the LDR)
				xf = read_float(in, bigendian);
				yf = read_float(in, bigendian);
				zf = read_float(in, bigendian);
				if (xf < 0.0) xf = 0.0, warn = true;
				if (yf < 0.0) yf = 0.0, warn = true;
				if (zf < 0.0) zf = 0.0, warn = true;
				//
				if (isnan(zf))
				{
					//print_error("Error when reading source file", 21);
				}
				//
				// Convert from XYZ to RGB (the same colorspace as the LDR)
				rf = xf * 3.2404542 + yf * -1.5371385 + zf * -0.4985314;
				gf = xf * -0.9692660 + yf * 1.8760108 + zf * 0.0415560;
				bf = xf * 0.0556434 + yf * -0.2040259 + zf * 1.0570000;
			}
			else
			{
				rf = read_float(in, bigendian);
				gf = read_float(in, bigendian);
				bf = read_float(in, bigendian);
				//
				if (rf < 0.0) rf = 0.0, warn = true;
				if (gf < 0.0) gf = 0.0, warn = true;
				if (bf < 0.0) bf = 0.0, warn = true;
				if (isnan(bf))
				{
					//print_error("Error when reading source file", 21);
				}
			}
			y = (0.2126 * rf + 0.7152 * gf + 0.0722 * bf);
		}
		else
		{
			int r, g, b;
			int max = (1l << count) - 1;
			// Integer samples, three components
			if (count <= 8)
			{
				r = getc(in);
				g = getc(in);
				b = getc(in);
			}
			else
			{
				r = getc(in) << 8;
				r |= getc(in);
				g = getc(in) << 8;
				g |= getc(in);
				b = getc(in) << 8;
				b |= getc(in);
			}
			if (b < 0)
			{
				//print_error("Error when reading source file", 21);
			}
			y = (0.2126 * r + 0.7152 * g + 0.0722 * b) / max;
			rf = static_cast<double>(r) / max;
			gf = static_cast<double>(g) / max;
			bf = static_cast<double>(b) / max;
			if (xyz)
			{
				double xf = r, yf = g, zf = b;
				// Convert from XYZ to RGB (the same colorspace as the LDR)
				rf = xf * 3.2404542 + yf * -1.5371385 + zf * -0.4985314;
				gf = xf * -0.9692660 + yf * 1.8760108 + zf * 0.0415560;
				bf = xf * 0.0556434 + yf * -0.2040259 + zf * 1.0570000;
				r = int(rf), g = int(gf), b = int(bf);
				if (r < 0.0) r = 0, warn = true;
				if (g < 0.0) g = 0, warn = true;
				if (b < 0.0) b = 0, warn = true;
				if (r > max) r = max, warn = true;
				if (g > max) g = max, warn = true;
				if (b > max) b = max, warn = true;
				y = (0.2126 * rf + 0.7152 * gf + 0.0722 * bf) / max;
			}
		}
	}
	else
	{
		int g;
		if (flt)
		{
			gf = read_float(in, bigendian);
			if (gf < 0.0) gf = 0.0, warn = true;
			y = gf;
		}
		else
		{
			if (count <= 8)
			{
				g = getc(in);
			}
			else
			{
				g = getc(in) << 8;
				g |= getc(in);
			}
			y = double(g) / ((1L << count) - 1);
		}
		rf = gf;
		bf = gf;
	}

	return warn;
}


double inline read_float(FILE* in, bool bigendian)
{
	LONG dt1, dt2, dt3, dt4;
	union
	{
		LONG long_buf;
		FLOAT float_buf;
	} u;

	dt1 = getc(in);
	dt2 = getc(in);
	dt3 = getc(in);
	dt4 = getc(in);

	if (dt4 < 0)
		return nan("");

	if (bigendian)
	{
		u.long_buf = (ULONG(dt1) << 24) | (ULONG(dt2) << 16) |
			(ULONG(dt3) << 8) | (ULONG(dt4) << 0);
	}
	else
	{
		u.long_buf = (ULONG(dt4) << 24) | (ULONG(dt3) << 16) |
			(ULONG(dt2) << 8) | (ULONG(dt1) << 0);
	}

	return u.float_buf;
}

UWORD inline double_to_half(double v)
{
	bool sign = (v < 0.0) ? (true) : (false);
	int exponent;
	int mantissa;

	if (v < 0.0) v = -v;

	if (isinf(v))
	{
		exponent = 31;
		mantissa = 0;
	}
	else if (v == 0.0)
	{
		exponent = 0;
		mantissa = 0;
	}
	else
	{
		double man = 2.0 * frexp(v, &exponent); // must be between 1.0 and 2.0, not 0.5 and 1.
		// Add the exponent bias.
		exponent += 15 - 1; // exponent bias
		// Normalize the exponent by modifying the mantissa.
		if (exponent >= 31)
		{
			// This must be denormalized into an INF, no chance.
			exponent = 31;
			mantissa = 0;
		}
		else if (exponent <= 0)
		{
			man *= 0.5; // mantissa does not have an implicit one bit.
			while (exponent < 0)
			{
				man *= 0.5;
				exponent++;
			}
			mantissa = int(man * (1 << 10));
		}
		else
		{
			mantissa = int(man * (1 << 10)) & ((1 << 10) - 1);
		}
	}

	return ((sign) ? (0x8000) : (0x0000)) | (exponent << 10) | mantissa;
}

double  half_to_double(UWORD h)
{
	bool sign = (h & 0x8000) ? (true) : (false);
	UBYTE exponent = (h >> 10) & ((1 << 5) - 1);
	UWORD mantissa = h & ((1 << 10) - 1);
	double v;

	if (exponent == 0)
	{
		// denormalized
		v = ldexp(float(mantissa), -14 - 10);
	}
	else if (exponent == 31)
	{
		v = HUGE_VAL;
	}
	else
	{
		v = ldexp(float(mantissa | (1 << 10)), -15 - 10 + exponent);
	}

	return (sign) ? (-v) : (v);
}






void write_ppm_file(FILE* fp, int width, int height, unsigned char* img) {
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	fwrite(img, sizeof(unsigned char), width * height * 3, fp);
}