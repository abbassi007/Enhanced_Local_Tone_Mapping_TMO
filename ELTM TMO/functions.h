#pragma once
#include <vector>
using namespace std;

vector<double> MeanFilter(const vector<double>& input, int width, int height, int radius);
vector<double> MeanFilterMovingSum(const vector<double>& input, int width, int height, int radius);
vector<double> operator+(const vector<double>& input1, const vector<double>& input2);
vector<double> operator+(const vector<double>& input1, const double input2);
vector<double> operator/(const vector<double>& input1, vector<double> input2);
vector<double> operator-(const vector<double>& input1, const vector<double>& input2);
vector<double> operator*(const vector<double>& input1, const vector<double>& input2);
vector<double> operator*(const vector<double>& input1, const double& input2);
vector<double> Clip(const vector<double>& input, const double& limit);


