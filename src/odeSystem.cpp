#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <unordered_map>
#include <regex>

#include <boost/numeric/odeint.hpp>

#include "include/odeSystem.h"

std::string ODESystem::parseVar(std::string &inp) {
	size_t pos = inp.find("var");
	if (pos == std::string::npos) {
		throw std::invalid_argument("Variable not found");
	}

	inp = inp.substr(pos + 4);	//remove 'var '

	int i = 0;
	std::string varName;
	for (char c : inp) {
		if (c == ' ') break;
		varName += c;
		i += 1;
	}
	inp = inp.substr(i+3);	//remove ' = '
	inp.pop_back();	//delete final ';'

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

void ODESystem::parseEmit(std::string &inp) {
	size_t pos = inp.find("emit");
	if (pos == std::string::npos) {
		throw std::invalid_argument("Emit not found");
	}

	inp = inp.substr(pos + 5);
	int i = 0;
	std::string val;
	for (char c : inp) {
		if (c == ' ') break;
		val += c;
		i += 1;
	}	//parse the value variable

	inp = inp.substr(i + 4); 	//remove ' as '
	std::string key;
	for (char c : inp) {
		if (c == ';') break;
		key += c;
	}

	global[key] = std::make_tuple(val, 0.0, 0.0);
}

void ODESystem::setScalars(ODE o) {
	for (size_t i = 0; i < o.interval.size(); i += 1) {
		o.varValues[i]->setScalar(o.interval[i]);
	}	
}	

int ODESystem::readODESystem(std::ifstream& inp, const bool scaled) {
	std::string line;

	std::regex system_r(R"(^\s*system\s+)");
	std::regex var_r(R"(^\s*var\s+)");
	std::regex interval_r(R"(^\s*interval\s+)");
	std::regex time_r(R"(^\s*time\s+)");
	std::regex emit_r(R"(^\s*emit\s+)");

	while (std::getline(inp, line)) {
		if (std::regex_search(line, system_r)) {
			ODE ode;
			while(std::getline(inp, line) && line != "}") {
				if (std::regex_search(line, var_r)) {
					try {
						ode.varNames.push_back(parseVar(line));
						Expr* e = new Expr();
						e->parse(line);
						ode.varValues.push_back(e);
					} catch (const std::logic_error &e) {
						std::cerr << "Error parsing var: " << e.what() << '\n';
						return 1;
					}
				}
				else if (std::regex_search(line, interval_r)) {
					try {
						ode.interval.push_back(parseInterval(line));
					} catch (const std::invalid_argument &e) {
						std::cerr << "Error parsing interval: " << e.what() << '\n';
						return 1;
					}
				}
				else if (std::regex_search(line, time_r)) {
					try {
						ode.time = parseTime(line);
					} catch (const std::invalid_argument &e) {
						std::cerr << "Error parsing time: " << e.what() << '\n';
						return 1;
					}
				}
				else if (std::regex_search(line, emit_r)) {
					try {
						parseEmit(line);
					} catch(const std::invalid_argument &e) {
						std::cerr << "Error parsing emit: " << e.what() << '\n';
					}
				}
			}
			
			// if -s was given as the command line argument set the scalars
			if (scaled) {
				setScalars(ode);
			}
			
			for (size_t i = 0; i < ode.interval.size(); i += 1) {
				std::cerr << "Var " << ode.varNames[i] << ' ' << ode.varValues[i]->getScalar() << '\n';
			}
			//Debug print the read ODE
			for (int i = 0; i < (int)ode.varNames.size(); i += 1) {
				std::cout << "<" << ode.varNames[i] << "><";
				ode.varValues[i]->print(); 
				std::cout << "> [" << ode.interval[i].first << ';' << ode.interval[i].second << "]\n";
			} 
			std::cout << ode.time << "\n\n";
			ODES.push_back(ode);
		}
	}

  for (auto& it : global) {
    std::string varName = std::get<0>(it.second);
    double initialValue = 0.0;
    double scalar = 0.0;

    // Find the initial value in the ODES
    for (const auto& ode : ODES) {
      auto itVar = std::find(ode.varNames.begin(), ode.varNames.end(), varName);
      if (itVar != ode.varNames.end()) {
        size_t index = std::distance(ode.varNames.begin(), itVar);
        initialValue = ode.varValues[index]->getInit();
        scalar = ode.varValues[index]->getScalar();
        break;
    	}
  	}
    it.second = std::make_tuple(varName, initialValue, scalar);
  }

	std::cout << "\nGlobal variables\n";
	for (const auto& it : global) {
		std::cout << it.first << ' ' << std::get<0>(it.second) << ' ' << std::get<1>(it.second) << ' ' << std::get<2>(it.second) << '\n';
	} std::cout << '\n';

	return 0;	
}