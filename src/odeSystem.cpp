#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>

#include "include/odeSystem.h"

std::string ODESystem::parseVar(std::string &inp) {
	while (inp.substr(0, 3) != "var") {
		inp = inp.substr(1);
	}
	inp = inp.substr(4);

	std::string varName;
	while (inp[0] != ' ') {
		varName += inp[0];
		inp = inp.substr(1);
	}
	inp = inp.substr(3, (int)inp.length()-4);

	return varName;
} 

std::pair<double, double> ODESystem::parseInterval(std::string &inp) {
	std::string buf1, buf2;
	while (inp[0] != '[') {
		inp = inp.substr(1);
	} inp = inp.substr(1);

	while (inp[0] != ',') {
		buf1 += inp[0];
		inp = inp.substr(1);
	} inp = inp.substr(1);

	while (inp[0] != ']') {
		buf2 += inp[0];
		inp = inp.substr(1);
	}

	return std::make_pair(std::stod(buf1), std::stod(buf2));
}

double ODESystem::parseTime(std::string &inp) {
	std::string buf;
	while (inp[0] != 'e') {
		inp = inp.substr(1);
	} inp = inp.substr(2);
	
	while (inp[0] != ';') {
		buf += inp[0];
		inp = inp.substr(1);
	}

	return std::stod(buf);
}

int ODESystem::readODESystem(std::ifstream& inp) {
	std::string line;

	while (std::getline(inp, line)) {
		if (line.find("system") != std::string::npos) {
			ODE ode;
			while(std::getline(inp, line) && line != "}") {
				if (line.find("var") != std::string::npos) {
					ode.varNames.push_back(parseVar(line));
					ode.varValues.push_back(line);
				}
				else if (line.find("interval") != std::string::npos) {
					ode.interval.push_back(parseInterval(line));
				}
				else if (line.find("time") != std::string::npos) {
					ode.time = parseTime(line);
				}
			}
			for (int i = 0; i < (int)ode.varNames.size(); i += 1) {
				std::cout << ode.varNames[i] << ' ' << ode.varValues[i] << " [" << ode.interval[i].first << ';' << ode.interval[i].second << "]\n";
			} 
			std::cout << ode.time << "\n\n";
			ODES.push_back(ode);
		}
	}

	return 0;	
}