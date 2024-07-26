#ifndef READODEH
#define READODEH

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>

#include "expression.h"

struct ODE {
	//Name of a variable
	std::vector<std::string> varNames;
	//Expression associated with a variable
	std::vector<Expr*> varValues;
	//Interval of each variable
	std::vector<std::pair<double, double>> interval;
	//Time duration of the ODE
	double time;
};

struct scalars {
	double rho;
	double delta;
};

class ODESystem {
public:
	~ODESystem() {
		for (int i = 0; i < (int)ODES.size(); i += 1) {
			for (auto& e: ODES[i].varValues) {
				delete e;
			}
		}
	}

	int readODESystem(std::ifstream& inp, 
										const bool scaled, 
										const bool clustering,
										const bool d);

	std::string parseVar(std::string& inp);
	std::pair<double, double> parseInterval(std::string &inp);
	double parseTime(std::string &inp);
	void parseEmit(std::string &inp);
	void setScalars(ODE o);

	void simulate();

	std::vector<var> extractConstants(const ODE& ode) const;
	std::vector<var> extractVariables(const ODE& ode) const;
	std::vector<global_var> extractGlobals() const;
	std::vector<Expr*> extractVariablesInteg(const ODE& ode) const;

	void parseFPAAOutput();
	bool setInpFileName(const std::string i);
	std::string getInpFileName();

	int editTreeDistance(const Node* root1, const Node* root2);
	std::vector<std::vector<int>> computeSimilarityMatrix(const std::vector<Expr*> vars);
	ODE cluster(ODE ode);

private:
	std::vector<ODE> ODES;
	std::unordered_map<std::string, std::tuple<std::string, double, scalars>> global;
	std::string systemName;
};

#endif