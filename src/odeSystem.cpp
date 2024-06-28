#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <utility>
#include <stdexcept>
#include <unordered_map>
#include <regex>

#include "include/odeSystem.h"

bool ODESystem::setInpFileName(const std::string i) {
	std::regex name_r(R"(^s*([^/]+)/([^.]+).ode)");
	std::smatch s;
	if (std::regex_search(i, s, name_r) && s.size() == 3) {
		systemName = s.str(2);
		return true;
	}
	return false;
}

std::string ODESystem::getInpFileName() {
	return systemName;
}

std::string ODESystem::parseVar(std::string &inp) {
	std::regex var_r(R"(^\s*var\s*([^\s]+)\s*=\s*([^;]+)\s*;)");
	std::smatch s;
	if (std::regex_search(inp, s, var_r) && s.size() == 3) {
		std::string ret = s.str(1);
		inp = s.str(2);
		return ret;
	}
	throw std::invalid_argument("Failed to parse variable\n");
} 

std::pair<double, double> ODESystem::parseInterval(std::string &inp) {
	std::regex interval_r(R"(^\s*interval\s*([^\s]+)\s*=\s*\[\s*([+-]?[0-9]*[.]?[0-9]+)\s*,\s*([+-]?[0-9]*\.?[0-9]+)\s*];)");
	std::smatch s;
	if (std::regex_search(inp, s, interval_r) && s.size() == 4) {
		return std::make_pair(std::stod(s[2]), std::stod(s[3]));
	}	
	throw std::invalid_argument("Failed to parse interval");
}

double ODESystem::parseTime(std::string &inp) {
	std::regex time_r(R"(^\s*time\s*([0-9]*\.?[0-9]+)\s*;)");
	std::smatch s;
	if (std::regex_search(inp, s, time_r) && s.size() == 2) {
		return std::stod((std::string)s[1]);
	}
	throw std::invalid_argument("Failed to parse time");
}

void ODESystem::parseEmit(std::string &inp) {
	std::regex emit_r(R"(^\s*emit\s*([^\s]+)\s*as\s*([^\s]+)\s*;)");
	std::smatch s; 
	if (std::regex_search(inp, s, emit_r) && s.size() == 3) {
		global[s[2]] = std::make_tuple(s[1], 0.0, 0.0);
	}
}

void ODESystem::setScalars(ODE o) {
	for (size_t i = 0; i < o.interval.size(); i += 1) {
		o.varValues[i]->setScalar(o.interval[i]);
	}	
}	

int ODESystem::readODESystem(std::ifstream& inp, const bool scaled, const bool d) {
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
						std::string x = parseVar(line);
						ode.varNames.push_back(x);
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
			if (d) {
				for (size_t i = 0; i < ode.varNames.size(); i += 1) {
					std::cerr << ode.varNames[i] << " = ";
					ode.varValues[i]->print(); 
					std::cerr << " [" << ode.interval[i].first << ";" << ode.interval[i].second << "]\n";
				}
				std::cerr << "time = " << ode.time << '\n';
			}
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
  if (d) {
  	for (auto& it : global) {
  		std::cerr << std::get<0>(it.second) << " emitted as " << it.first << " with " << std::get<1>(it.second) << '\n';
  	}
  }

	return 0;	
}