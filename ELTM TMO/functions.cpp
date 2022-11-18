#include "functions.h"
#include <stdlib.h>
#include <stdio.h>
#include <cmath>

vector<double> MeanFilter(const vector<double>& input, int width, int height, int radius)
{
	vector<double> output;
	radius -= 1;
	int limitUp, limitDown, limitLeft, limitRight;
	int sum, N;
	for (int i = 0; i < height; i++)
	{
		limitUp = ((i - radius) > 0) ? (i - radius) : 0;
		limitDown = ((i + radius) < height - 1) ? (i + radius) : height - 1;

		for (int j = 0; j < width; j++)
		{
			sum = 0;
			N = 0;

			limitLeft = ((j - radius) > 0) ? (j - radius) : 0;
			limitRight = ((j + radius) < width - 1) ? (j + radius) : width - 1;

			for (int x = limitUp; x <= limitDown; x++)
			{
				for (int y = limitLeft; y <= limitRight; y++)
				{
					sum += input.at(x * width + y);
					N++;
				}
			}

			if (N > 0)
			{
				sum /= N;
				output.push_back(sum);
			}
			else
			{
				fprintf(stderr, "**** Value of width, height and radius is invalid\n");
				exit(20);
			}
		}
	}
	return output;
}



vector<double> MeanFilterMovingSum(const vector<double>& input, int width, int height, int radius)
{
	vector<double> output, sum_temp;
	int N = (2 * radius + 1) * (2 * radius + 1);
	//Calculate sum for column
	//Init for first row
	for (int j = 0; j < width; j++)
	{
		double sum = 0;
		for (int p = -radius; p <= radius; p++)
		{
			int temp_p = (p < 0) ? 0 : p;
			temp_p = (temp_p >= height) ? (height - 1) : temp_p;
			sum += input.at(temp_p * width + j);
		}
		sum_temp.push_back(sum);
	}
	//Calculate others using moving sum
	for (int i = 1; i < height; i++)
	{
		int add_index = (i + radius) > (height - 1) ? height - 1 : i + radius;
		int sub_index = (i - radius - 1) < 0 ? 0 : i - radius - 1;
		for (int j = 0; j < width; j++)
		{
			double sum = sum_temp.at((i - 1) * width + j) + input.at(add_index * width + j) - input.at(sub_index * width + j);
			sum_temp.push_back(sum);
		}
	}

	//Calculate sum for row
	//Init for first column
	for (int i = 0; i < height; i++)
	{

	}
	//Calculate others using moving sum
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (j == 0)
			{
				double sum = 0;
				for (int p = -radius; p <= radius; p++)
				{
					int temp_p = (p < 0) ? 0 : p;
					temp_p = (temp_p >= width) ? (width - 1) : temp_p;
					sum += sum_temp.at(i * width + temp_p);
				}
				output.push_back(sum / N);
			}
			else
			{
				int add_index = (j + radius) > (width - 1) ? width - 1 : j + radius;
				int sub_index = (j - radius - 1) < 0 ? 0 : j - radius - 1;
				double sum = output.at(i * width + j - 1) + sum_temp.at(i * width + add_index) / N - sum_temp.at(i * width + sub_index) / N;
				output.push_back(sum);
			}
		}
	}
	return output;
}


vector<double> operator+(const vector<double>& input1, const vector<double>& input2)
{
	if (input1.size() == input2.size()) {

		int size = input1.size();
		vector<double> result;
		for (int i = 0; i < size; i++)
		{
			result.push_back(input1.at(i) + input2.at(i));
		}
		return result;
	}
	else
	{
		fprintf(stderr, "Vector size must be same:\n");
		exit(20);
	}
	
	
}
vector<double> operator+(const vector<double>& input1, const double input2)
{
	int size = input1.size();
	vector<double> result;
	for (int i = 0; i < size; i++)
	{
		result.push_back(input1.at(i) + input2);
	}
	return result;
}
vector<double> operator/(const vector<double>& input1, vector<double> input2)
{
	if (input1.size() == input2.size())
	{
		int size = input1.size();
		vector<double> result;
		for (int i = 0; i < size; i++)
		{
			result.push_back(input1.at(i) / input2.at(i));
		}
		return result;
	}
	else
	{
		fprintf(stderr, "Vector size must be same:\n");
		exit(20);
	}
}
vector<double> operator-(const vector<double>& input1, const vector<double>& input2)
{
	if (input1.size() == input2.size())
	{
		int size = input1.size();
		vector<double> result;
		for (int i = 0; i < size; i++)
		{
			result.push_back(input1.at(i) - input2.at(i));
		}
		return result;
	}
	else
	{
		fprintf(stderr, "Vector size must be same:\n");
		exit(20);
	}
}
vector<double> operator*(const vector<double>& input1, const vector<double>& input2)
{
	if (input1.size() == input2.size())
	{
		int size = input1.size();
		vector<double> result;
		for (int i = 0; i < size; i++)
		{
			result.push_back(input1.at(i) * input2.at(i));
		}
		return result;
	}
	else
	{
		fprintf(stderr, "Vector size must be same:\n");
		exit(20);
	}
}
vector<double> operator*(const vector<double>& input1, const double& input2)
{
	int size = input1.size();
	vector<double> result;
	for (int i = 0; i < size; i++)
	{
		result.push_back(input1.at(i) * input2);
	}
	return result;
}

vector<double> Clip(const vector<double>& input, const double& limit)
{
	int size = input.size();
	vector<double> output;
	for (int i = 0; i < size; i++)
	{
		if (fabs(input.at(i)) > limit)
		{
			if (input.at(i) > 0)
			{
				output.push_back(limit);
			}
			else
			{
				output.push_back(-limit);
			}
		}
		else
		{
			output.push_back(input.at(i));
		}
	}
	return output;
}



