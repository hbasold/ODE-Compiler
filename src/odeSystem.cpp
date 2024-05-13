#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <sstream>
#include <utility>
#include <stdexcept>
#include <unordered_map>

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

	global[key] = val;
}

void ODESystem::setScalars(ODE o) {
	for (size_t i = 0; i < o.interval.size(); i += 1) {
		o.varValues[i]->setScalar(o.interval[i]);
	}	
}	

int ODESystem::readODESystem(std::ifstream& inp, const bool scaled) {
	std::string line;

	while (std::getline(inp, line)) {
		if (line.find("system") != std::string::npos) {
			ODE ode;
			while(std::getline(inp, line) && line != "}") {
				if (line.find("var") != std::string::npos) {
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
				else if (line.find("interval") != std::string::npos) {
					try {
						ode.interval.push_back(parseInterval(line));
					} catch (const std::invalid_argument &e) {
						std::cerr << "Error parsing interval: " << e.what() << '\n';
						return 1;
					}
				}
				else if (line.find("time") != std::string::npos) {
					try {
						ode.time = parseTime(line);
					} catch (const std::invalid_argument &e) {
						std::cerr << "Error parsing time: " << e.what() << '\n';
						return 1;
					}
				}
				else if (line.find("emit") != std::string::npos) {
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

	for (const auto& it : global) {
		std::cout << it.first << ' ' << it.second << '\n';
	}

	return 0;	
}

std::vector<var> ODESystem::extractConstants(const ODE& ode) const {
	std::vector<var> constants;
	for (size_t i = 0; i < ode.varNames.size(); ++i) {
	  if (ode.varNames[i] != "time" && ode.interval[i].first == ode.interval[i].second) {
	    var v;
	    v.name = ode.varNames[i];
	    v.value = ode.varValues[i]->getInit();
	    v.scalar = ode.varValues[i]->getScalar();
	  	constants.push_back(v);
	  }
	}
	return constants;
}

std::vector<var> ODESystem::extractVariables(const ODE& ode) const {
  std::vector<var> variables;
  for (size_t i = 0; i < ode.varNames.size(); ++i) {
    if (ode.varNames[i] != "time" && ode.interval[i].first != ode.interval[i].second) {
    	var v;
    	v.name = ode.varNames[i];
    	v.value = ode.varValues[i]->getInit();
    	v.scalar = ode.varValues[i]->getScalar();
      variables.push_back(v);
    }
  }
  return variables;
}

std::vector<Expr*> extractVariablesInteg(const ODE& ode) {
  std::vector<Expr*> variables;
  for (size_t i = 0; i < ode.varValues.size(); ++i) {
    if (ode.varValues[i]->isInteg()) {
      variables.push_back(ode.varValues[i]);
    }
  }
  return variables;
}


/*
*	Simulate the system of ODEs using the odeint library
*/

class IntegrationObserver {
public:
    IntegrationObserver(std::ofstream& outfile) : output(outfile) {}

    template<typename State>
    void operator()(const State& x, double t) const {
        output << t << ",";
        for (size_t i = 0; i < x.size(); ++i) {
            output << x[i] << ",";
        }
        output << std::endl;
    }

private:
    std::ofstream& output;
};

struct ODEs {
  const std::vector<Expr*>& expressions;
  const std::vector<var>& constants;
  std::vector<var>& variables;

  ODEs(const std::vector<Expr*>& exprs, 
    const std::vector<var>& consts,
    std::vector<var>& vars)
    : expressions(exprs), constants(consts), variables(vars) {}

  void operator()(const std::vector<double>& x, std::vector<double>& dxdt, const double /* t */) const {
		for (size_t i = 0; i < x.size(); i += 1) {
    	variables[i].value = x[i];
    } 
    // Evaluate each expression in the system of ODEs
    for (size_t i = 0; i < expressions.size(); ++i) {
      // Evaluate the expression and assign the result to the corresponding dxdt element
      dxdt[i] = expressions[i]->Evaluate(constants, variables);
    }
  }
};

void ODESystem::simulate() {
	using namespace boost::numeric::odeint;

	//for every ODE
	for (auto &it : ODES) {
		auto constants = extractConstants(it);
		auto vars = extractVariables(it);

		auto varExpr = extractVariablesInteg(it);

		std::vector<double> x(vars.size());

		size_t id = 0;
		for (const auto &i : varExpr) {
			x[id++] = i->getInit();
		}

		for (const auto& i : vars) {
			std::cout << i.name << ' ' << i.value << ' ' << i.scalar << '\n';
		}

		for (size_t i = 0; i < x.size(); i += 1) {
			std::cout << x[i] << ' ';
		} std::cout << '\n';

		auto stepper = runge_kutta4<std::vector<double>>();

		std::ofstream outputFile("res/outp.csv");
		if (!outputFile.is_open()) {
			std::cerr << "Can't open outputfile\n";
			return;
		}

		auto observer = IntegrationObserver(outputFile);
		outputFile << "Time,x,y,z,\n";

		integrate_const(stepper, ODEs(varExpr, constants, vars), x, 0.0, it.time, 0.01, observer);

		outputFile.close();

    // Output the results
    size_t idx = 0;
    for (const auto& entry : vars) {
        std::cout << entry.name << " = " << x[idx++] << std::endl;
    }
	}
}