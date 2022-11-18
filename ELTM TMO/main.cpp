#include"type.h"
#include <iostream>
#include"eltmtmo.h"
#include"constant.h"
#include"functions.h"

#include <string.h>
#include <limits>

#define ARG_EPSILON   "-eps"
#define ARG_RS        "-rs"
#define ARG_EPSILON_S "-eps-s"
#define ARG_EPSILON_L "-eps-l"
#define ARG_LAMBDA_F  "-lambda-f"
#define ARG_LAMBDA_C  "-lambda-c"
#define ARG_TAU_R     "-tau-r"
#define ARG_ETA_F     "-eta-f"
#define ARG_ETA_C     "-eta-c"
#define ARG_P         "-p"
#define ARG_K         "-k"
#define ARG_M         "-m"
#define ARG_BPC_MAX   "-bpc-max"
#define ARG_BPC_MIN   "-bpc-min"
#define ARG_S         "-s"

struct Args
{
	ELTM_Params params;
	const char* input_path;
	const char* output_path;
};
bool parse_args(Args& parsed_args, int argc, const char** argv);
std::ostream& operator<<(std::ostream& os, const Args& args);

int main(int argc, const char *argv[]) {
	int width, height, depth, precision;
	bool pfm, big_endian;
	double gamma = 2.2;

	Args parsed_args;
	if (!parse_args(parsed_args, argc, argv))
		return -1;

	const char* input_file = parsed_args.input_path;
	const char* output_file = parsed_args.output_path;
	const ELTM_Params& params = parsed_args.params;

	//Read HDR Image
	FILE* hdr_in = open_PNM(input_file, width, height, depth, precision, pfm, big_endian);

	//initialize memory for the output
	MEMORY* ldr_raw_buffer =  (MEMORY*)malloc(width * height * depth);

	//start process of tmo to convert HDR image to LDR image.
	eltm_tmo(hdr_in, width, height, precision,depth,ldr_raw_buffer, gamma, pfm, big_endian, params);

	//open file for LDR image
	FILE* fp = fopen(output_file, "wb");

	//writing LDR image 
	write_ppm_file(fp, width, height, ldr_raw_buffer);
	fclose(fp);
	free(ldr_raw_buffer);

	return 0;
}

void print_help(const char* program_name);
const char* get_program_name(const char* exe_path);

bool parse_args(Args& parsed_args, int argc, const char** argv)
{
	// Check for minimum number of args
	if (argc < 2)
	{
		std::cout << "Missing required args" << std::endl;
		print_help(get_program_name(argv[0]));
		return false;
	}

	enum class ParserState
	{
		kExpectArg,
		kExpectInt,
		kExpectDouble
	} state = ParserState::kExpectArg;
	int positional_arg = 0;

	int* pIntValue;
	double* pDoubleValue;

	for (int i = 1; i < argc; ++i)
	{
		const char* arg = argv[i];
		switch (state)
		{
		case ParserState::kExpectArg:
		{
			if (strcmp(arg, "-help") == 0)
			{
				print_help(get_program_name(argv[0]));
				return false;
			}
			else if (strcmp(arg, ARG_BPC_MAX) == 0)
			{
				pDoubleValue = &parsed_args.params.BPCmax;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_BPC_MIN) == 0)
			{
				pDoubleValue = &parsed_args.params.BPCmin;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_EPSILON) == 0)
			{
				pDoubleValue = &parsed_args.params.epsilon;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_EPSILON_L) == 0)
			{
				pDoubleValue = &parsed_args.params.epsilonL;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_EPSILON_S) == 0)
			{
				pDoubleValue = &parsed_args.params.epsilonS;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_ETA_C) == 0)
			{
				pDoubleValue = &parsed_args.params.etaC;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_ETA_F) == 0)
			{
				pDoubleValue = &parsed_args.params.etaF;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_K) == 0)
			{
				pDoubleValue = &parsed_args.params.k;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_LAMBDA_C) == 0)
			{
				pDoubleValue = &parsed_args.params.lamdaC;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_LAMBDA_F) == 0)
			{
				pDoubleValue = &parsed_args.params.lamdaF;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_M) == 0)
			{
				pDoubleValue = &parsed_args.params.m;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_P) == 0)
			{

				pDoubleValue = &parsed_args.params.p;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_RS) == 0)
			{
				pIntValue = &parsed_args.params.rS;
				state = ParserState::kExpectInt;
			}
			else if (strcmp(arg, ARG_S) == 0)
			{
				pDoubleValue = &parsed_args.params.s;
				state = ParserState::kExpectDouble;
			}
			else if (strcmp(arg, ARG_TAU_R) == 0)
			{
				pIntValue = &parsed_args.params.tauR;
				state = ParserState::kExpectInt;
			}
			else
			{
				// Positional args
				if (positional_arg == 0)
					parsed_args.input_path = arg;
				else if (positional_arg == 1)
					parsed_args.output_path = arg;
				else
				{
					std::cout << "Received invalid positional argument " << arg << std::endl;
					return false;
				}
				++positional_arg;
			}
		} break;
		case ParserState::kExpectInt:
		{
			char* end;
			const long value = strtol(arg, &end, 10);
			if (end[0] != '\0')
			{
				std::cout << "Expected integer value. Received " << arg << std::endl;
				return false;
			}
			if (value < -std::numeric_limits<int>::max() || value > std::numeric_limits<int>::max())
			{
				std::cout << "Integer value " << value << " out of range" << std::endl;
				std::cout << "Valid range [" << -std::numeric_limits<int>::max() << ", " << std::numeric_limits<int>::max() << ']' << std::endl;
				return false;
			}
			*pIntValue = static_cast<int>(value);
			state = ParserState::kExpectArg;
		} break;
		case ParserState::kExpectDouble:
		{
			char* end;
			*pDoubleValue = strtod(arg, &end);
			if (end[0] != '\0')
			{
				std::cout << "Expected double value. Received " << arg << std::endl;
				return false;
			}
			state = ParserState::kExpectArg;
		} break;
		default:
			break;
		}
	}
	return true;
}

std::ostream& operator<<(std::ostream& os, const Args& args)
{
	os << "Positional args:\n";
	os << "\tinput: " << args.input_path << '\n';
	os << "\toutput: " << args.output_path << '\n';
	os << "Optional args:\n";
	os << "\tepsilon: " << args.params.epsilon << '\n';
	os << "\trS: " << args.params.rS << '\n';
	os << "\tepsilonS: " << args.params.epsilonS << '\n';
	os << "\tepsilonL: " << args.params.epsilonL << '\n';
	os << "\tlambdaF: " << args.params.lamdaF << '\n';
	os << "\tlambdaC: " << args.params.lamdaC << '\n';
	os << "\ttauR: " << args.params.tauR << '\n';
	os << "\tetaF: " << args.params.etaF << '\n';
	os << "\tetaC: " << args.params.etaC << '\n';
	os << "\tp: " << args.params.p << '\n';
	os << "\tk: " << args.params.k << '\n';
	os << "\tm: " << args.params.m << '\n';
	os << "\tBPCmax: " << args.params.BPCmax << '\n';
	os << "\tBPCmin: " << args.params.BPCmin << '\n';
	os << "\ts: " << args.params.s << '\n';
	return os;
}

void print_help(const char* program_name)
{
	std::cout << "Usage: " << program_name << " INPUT OUTPUT ";
	std::cout << "[" ARG_EPSILON   " epsilon] ";
	std::cout << "[" ARG_RS        " rS] ";
	std::cout << "[" ARG_EPSILON_S " epsilonS] ";
	std::cout << "[" ARG_EPSILON_L " epsilonL] ";
	std::cout << "[" ARG_LAMBDA_F  " lambdaF] ";
	std::cout << "[" ARG_LAMBDA_C  " lambdaC] ";
	std::cout << "[" ARG_TAU_R     " tauR] ";
	std::cout << "[" ARG_ETA_F     " etaF] ";
	std::cout << "[" ARG_ETA_C     " etaC] ";
	std::cout << "[" ARG_P         " p] ";
	std::cout << "[" ARG_K         " k] ";
	std::cout << "[" ARG_M         " m] ";
	std::cout << "[" ARG_BPC_MAX   " BPCmax] ";
	std::cout << "[" ARG_BPC_MIN   " BPCmin] ";
	std::cout << "[" ARG_S         " s] ";
}

const char* get_program_name(const char* exe_path)
{
#if defined(_WIN32)
	const char sep = '\\';
#else
	const char sep = '/';
#endif
	const char* name = exe_path;
	while (true)
	{
		const char *found = strchr(name, sep);
		if (found)
		{
			name = found + 1;
		}
		else
			break;
	}
	return name;
}

