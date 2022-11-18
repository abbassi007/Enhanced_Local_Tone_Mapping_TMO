#include "eltmtmo.h"
#include<chrono>
#include "edgeAwareFilter.h"
#include"functions.h"
#include <algorithm> 
#include <numeric>
#include <iostream>
#include <math.h>

void eltm_tmo(FILE* in, int w, int h, int count, int depth, MEMORY* image_buffer, double gamma,
	bool flt, bool bigendian, const ELTM_Params& params)
{
	auto start = std::chrono::high_resolution_clock::now();

	long pos = ftell(in);
	int size = w * h;

	const int rL = (w < h) ? w / 10 : h / 10; //the large guided filter's radius (rL) was calculated as 10% of the min (height, width)

	vector<double> Ylog;
	vector<double> DPFlog, DPClog, BPFlog, BPlog;
	double maxBPlog, minBPlog;
	double alpha, beta;
	double SG;
	double DPFlog_, DPClog_, BPlog_;
	double BP, DP;
	double BPmin, BPmax;
	double BPC, p_, YC;

	/***********************************************************************
	 *                       LOG DOMAIN PROCESSING                         *
	 ***********************************************************************/

	int x, y, i;
	for (y = 0; y < h; y++)
	{
		for (x = 0; x < w; x++)
		{
			double y;

			/*------------Exact luminance channel------------*/
			Read_Y_Comp(in, y, count, depth, flt, bigendian, false);
			Ylog.push_back(log2(y + params.epsilon));
		}
	}
	//Reset file position of the stream to begining of file;
	fseek(in, pos, SEEK_SET);

	/*----------------3.2. Decomposition----------------*/
	DPFlog = Clip(Ylog - edgeAwareFilter(Ylog, w, h, params.rS, params.epsilonS), params.lamdaF);
	BPFlog = Ylog - DPFlog;
	//Delete Ylog
	vector<double>().swap(Ylog);

	DPClog = Clip(BPFlog - edgeAwareFilter(BPFlog, w, h, rL, params.epsilonL), params.lamdaC);
	BPlog = BPFlog - DPClog;
	//Delete BPFlog
	vector<double>().swap(BPFlog);

	/*-----------3.3. Logarithm domain contrast reduction------------*/
	maxBPlog = *max_element(BPlog.begin(), BPlog.end());
	minBPlog = *min_element(BPlog.begin(), BPlog.end());

	beta = -maxBPlog;
	alpha = (double)params.tauR / (maxBPlog - minBPlog);

	BPlog = (BPlog + beta) * alpha; //BP'log (5)

	/*----------------3.4. Detail enhancement-----------------*/
	//Calculate min and max of BP'log. Because alpha always > 0
	minBPlog = (minBPlog + beta) * alpha;
	maxBPlog = (maxBPlog + beta) * alpha;


	/***********************************************************************
	 *                      LINEAR DOMAIN PROCESSING                       *
	 ***********************************************************************/
	//Calculate BPmin and BPmax for function (9). Because y = 2^x is an increasing function, so
	BPmin = exp2(minBPlog);
	BPmax = exp2(maxBPlog);

	/*--------------5. ELTM brightness control-------------------*/
	p_ = params.p * pow(10, params.k * (accumulate(BPlog.begin(), BPlog.end(), 0.0) / BPlog.size() - params.m));

	//Start computing loop from function (6)

	MEMORY* image_buffer_temp = image_buffer;
	for (x = 0; x < size; x++)
	{
		
		//Function (6)
		SG = (-0.4 * BPlog.at(x) > 1) ? (-0.4 * BPlog.at(x)) : 1;
		DPFlog_ = params.etaF * SG * DPFlog.at(x); 
		DPClog_ = params.etaC * SG * DPClog.at(x); 
		//Function (7)
		BP = exp2(BPlog.at(x));
		DP = exp2(DPFlog_ + DPClog_);

		/*--------------3.5. Tone compression---------------------*/
		BPC = (params.BPCmax - params.BPCmin) * (log((BP - BPmin) / (BPmax - BPmin) + p_) - log(p_)) / (log(1 + p_) - log(p_)) + params.BPCmin;
		YC = BPC * DP;

		/*----------------3.6. Color restoration------------------*/
		double RIn, GIn, BIn;
		double ROut, GOut, BOut;
		double YIn;

		Read_RGB_Comp(in, RIn, GIn, BIn, YIn, count, depth, flt, bigendian, false);
		ROut = YC * pow(RIn / YIn, params.s / gamma);
		GOut = YC * pow(GIn / YIn, params.s / gamma);
		BOut = YC * pow(BIn / YIn, params.s / gamma);

		//Build histogram
		//int r,g,b;
		int rl, gl, bl;

		
		rl = round(ROut * 255);
		gl = round(GOut * 255);
		bl = round(BOut * 255);

	

		rl = (rl > 255) ? 255 : rl;
		gl = (gl > 255) ? 255 : gl;
		bl = (bl > 255) ? 255 : bl;
		rl = (rl < 0) ? 0 : rl;
		gl = (gl < 0) ? 0 : gl;
		bl = (bl < 0) ? 0 : bl;

		*image_buffer_temp = (MEMORY)rl;
		image_buffer_temp++;
		*image_buffer_temp = (MEMORY)gl;
		image_buffer_temp++;
		*image_buffer_temp = (MEMORY)bl;
		image_buffer_temp++;

		

	}
	//Delete Ylog
	vector<double>().swap(DPFlog);
	vector<double>().swap(DPClog);
	vector<double>().swap(BPlog);

	fseek(in, pos, SEEK_SET);
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	cout << "Time taken by ELTM: " << duration.count() << " milliseconds" << endl;
}