#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <unordered_map>

#include <boost/numeric/odeint.hpp>

#include "include/odeSystem.h"
#include "include/constants.h"

std::vector<var> ODESystem::extractConstants(const ODE& ode) const {
	std::vector<var> constants;
	for (size_t i = 0; i < ode.varNames.size(); ++i) {
	  if (ode.varNames[i] != "time" && ode.interval[i].first == ode.interval[i].second) {
	    var v;
	    v.name = ode.varNames[i];
	    v.value = ode.varValues[i]->getInit();
	    v.rho = ode.varValues[i]->getRho();
      v.delta = ode.varValues[i]->getDelta();
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
      v.rho = ode.varValues[i]->getRho();
      v.delta = ode.varValues[i]->getDelta();
      variables.push_back(v);
    }
  }
  return variables;
}

std::vector<Expr*> ODESystem::extractVariablesInteg(const ODE& ode) const {
  std::vector<Expr*> variables;
  for (size_t i = 0; i < ode.varValues.size(); ++i) {
    if (ode.varValues[i]->isInteg()) {
      variables.push_back(ode.varValues[i]);
    }
  }
  return variables;
}

std::vector<global_var> ODESystem::extractGlobals() const {
	std::vector<global_var> globals;
	for (const auto& it : global) {
		global_var v;
		v.name = it.first;
		v.local_name = std::get<0>(it.second);
		v.value = std::get<1>(it.second);
		v.rho = std::get<2>(it.second).rho;
    v.delta = std::get<2>(it.second).delta;
		globals.push_back(v);
	}
	return globals;
}


/*
*	Simulate the system of ODEs using the odeint library
*/

class IntegrationObserver {
public:
    IntegrationObserver(std::ofstream& outfile) : output(outfile) {}

    template<typename State>
    void operator()(const State& x, double t) const {
        for (size_t i = 0; i < x.size(); i += 1) {
            output << x[i] << ",";
        }
    }

private:
    std::ofstream& output;
};

struct ODEs {
  const std::vector<Expr*>& expressions;
  const std::vector<var>& constants;
  std::vector<var>& variables;
  std::vector<global_var>& globals;

  ODEs(const std::vector<Expr*>& exprs, 
    const std::vector<var>& consts,
    std::vector<var>& vars,
    std::vector<global_var>& global)
    : expressions(exprs), constants(consts), variables(vars), globals(global) {}

  void operator()(const std::vector<double>& x, std::vector<double>& dxdt, const double /* t */) const {
		for (size_t i = 0; i < x.size(); i += 1) {
    	variables[i].value = x[i];
    } 
    // Evaluate each expression in the system of ODEs
    for (size_t i = 0; i < expressions.size(); ++i) {
      // Evaluate the expression and assign the result to the corresponding dxdt element
      dxdt[i] = expressions[i]->Evaluate(constants, variables, globals);
    }
  }
};

void ODESystem::simulate() {
  using namespace boost::numeric::odeint;

  std::vector<std::vector<double>> stateVectors;
  std::vector<std::vector<var>> variableSets;
  std::vector<std::vector<Expr*>> expressionSets;
  std::vector<std::vector<var>> constantSets;
  auto global = extractGlobals();

  for (auto &it : ODES) {
    auto constants = extractConstants(it);
    auto vars = extractVariables(it);
    auto varExpr = extractVariablesInteg(it);

    std::vector<double> x(vars.size());
    size_t id = 0;
    for (const auto &i : varExpr) {
       x[id++] = i->getInit();
    }

    stateVectors.push_back(x);
    variableSets.push_back(vars);
    expressionSets.push_back(varExpr);
    constantSets.push_back(constants);
  }
  std::string outputFileName = "res/" + systemName + ".csv";
  std::ofstream outputFile(outputFileName);
  if (!outputFile.is_open()) {
    std::cerr << "Can't open outputfile\n";
    return;
	}

	outputFile << "time,";
	for (const auto& g : global) {
		outputFile << g.name << ',';
	} outputFile << '\n';

  auto stepper = runge_kutta4<std::vector<double>>();

  for (double time = 0; time < ODES[0].time; time += STEPPER) {
  	outputFile << time << ',';
    for (size_t i = 0; i < ODES.size(); ++i) {
      integrate_const(stepper, ODEs(expressionSets[i], constantSets[i], variableSets[i], global), stateVectors[i], time, time + STEPPER, STEPPER);
    	
    	for (const auto &v : variableSets[i]) {
      	for (auto& g : global) {
      		if (g.local_name == v.name) {
      			g.value = v.value;
      			outputFile << g.value << ',';
      		}
      	}
      }
    }
    outputFile << '\n';
  }
  outputFile.close();
}