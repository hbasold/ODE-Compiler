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

//Function which parses the ODE-system into an FPAA config
void ODESystem::parseFPAAOutput() {
	std::string name = "FPAAres/" + systemName + ".FPAAconfig";
	std::ofstream outputFile(name);
	int c = 0;
	
	std::vector<var> constants;
	std::vector<var> variables;
	std::vector<global_var> globals = extractGlobals();
	for (auto &o : ODES) {
		constants = extractConstants(o);
		variables = extractVariables(o);

		for (size_t i = 0; i < o.varValues.size(); i += 1) {
			if (o.varValues[i]->isInteg()) {
				outputFile << "#FPAA Config for expression of variable " << o.varNames[i] << "\nFPAASystem_" << c << " {\n";
				o.varValues[i]->FPAAPrintConfig(outputFile, c, constants, variables, globals, o.varNames[i]);
				outputFile << "};\n\n";
				c += 1;
			}
		} 
	}
	outputFile.close();
}