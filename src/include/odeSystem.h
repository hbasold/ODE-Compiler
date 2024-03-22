#ifndef READODEH
#define READODEH

#include <vector>
#include <string>
#include <tuple>

#include "expression.h"

struct ODE {
	//Name of a variable
	std::vector<std::string> varNames;
	//Expression associated with a variable
	std::vector<std::string> varValues;
	/*
	TODO: Add EMIT functionality
	*/
	//Interval of each variable
	std::vector<std::pair<double, double>> interval;
	//Time duration of the ODE
	double time;
};

class ODESystem {
public:
	int readODESystem(std::ifstream& inp);

	std::string parseVar(std::string& inp);
	std::pair<double, double> parseInterval(std::string &inp);
	double parseTime(std::string &inp);
private:
	std::vector<ODE> ODES;
};

#endif