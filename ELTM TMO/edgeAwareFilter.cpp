#include"edgeAwareFilter.h"
#include"functions.h"

vector<double> edgeAwareFilter(const vector<double>& p, int width, int height, int radius, double epsilon)
{
	/* K. He, J. Sun, X. Tang, Guided image filtering, IEEE Trans. Pattern Anal. Mach.
	Intell. 35 (6) (2013) 1397–1409*/

	/* input image I and guidance image p are identical */
	vector<double> meanI, corrI, varI, a, meanA, meanB;
	vector<double> q;
	//Step 1:
	meanI = MeanFilterMovingSum(p, width, height, radius);
	corrI = MeanFilterMovingSum(p * p, width, height, radius);
	//Step 2:

	varI = corrI - (meanI * meanI);
	//Delete corrI
	corrI.clear();
	

	//Step 3&4:
	a = varI / (varI + epsilon);
	//Delete varI
	varI.clear();

	meanA = MeanFilterMovingSum(a, width, height, radius);

	meanB = MeanFilterMovingSum(meanI - (a * meanI), width, height, radius);

	//Delete meanI
	meanI.clear();
	//Delete a
	a.clear();

	//Step 5:
	q = (meanA * p) + meanB;

	return q;
}


