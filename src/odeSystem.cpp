#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>

#include "include/odeSystem.h"

std::string ODESystem::parseVar(std::string &inp) {
	size_t pos = inp.find("var");
	if (pos == std::string::npos) {
		throw std::invalid_argument("Variable not found");
	}

	inp = inp.substr(pos + 4);

	int i = 0;
	std::string varName;
	for (char c : inp) {
		if (c == ' ') break;
		varName += c;
		i += 1;
	}
	inp = inp.substr(i + 3);

	return varName;
} 

std::pair<double, double> ODESystem::parseInterval(std::string &inp) {
	size_t start = inp.find('[');
	size_t comma = inp.find(',');
	size_t end = inp.find(']');

	if (start == std::string::npos || comma == std::string::npos || end == std::string::npos) {
		throw std::invalid_argument("Interval not correctly formatted");
	}

	std::string buf1 = inp.substr(start + 1, comma - start - 1);
	std::string buf2 = inp.substr(comma + 1, end - comma - 1);
	std::pair<double, double> res;
	try {
		res = std::make_pair(std::stod(buf1), std::stod(buf2));
	} catch(std::invalid_argument &e) {
		throw std::invalid_argument("Failed to parse interval");
	}

	return res;
}

double ODESystem::parseTime(std::string &inp) {
	size_t pos = inp.find("time");
	if (pos == std::string::npos) {
		throw std::invalid_argument("Time not found");
	}

	inp = inp.substr(pos + 5);

	std::string buf;
	for (char c : inp) {
		if (c == ';') break;
		buf += c;
	}

	double res;
	try {
		res = std::stod(buf);
	} catch (std::invalid_argument &e) {
		throw std::invalid_argument("Failed to parse time");
	}

	return res;
}

int ODESystem::readODESystem(std::ifstream& inp) {
	std::string line;

	while (std::getline(inp, line)) {
		if (line.find("system") != std::string::npos) {
			ODE ode;
			while(std::getline(inp, line) && line != "}") {
				if (line.find("var") != std::string::npos) {
					try {
						ode.varNames.push_back(parseVar(line));
						ode.varValues.push_back(line);
					} catch (const std::logic_error &e) {
						std::cerr << "Error: " << e.what() << '\n';
						return 1;
					}
				}
				else if (line.find("interval") != std::string::npos) {
					try {
						ode.interval.push_back(parseInterval(line));
					} catch (const std::invalid_argument &e) {
						std::cerr << "Error: " << e.what() << '\n';
						return 1;
					}
				}
				else if (line.find("time") != std::string::npos) {
					try {
						ode.time = parseTime(line);
					} catch (const std::invalid_argument &e) {
						std::cerr << "Error: " << e.what() << '\n';
						return 1;
					}
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