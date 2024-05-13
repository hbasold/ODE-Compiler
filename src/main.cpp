#include <iostream>
#include <fstream>

#include "include/odeSystem.h"

int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cerr << "Compiler usage: " << argv[0] << " -s/n <filename.ode>\n";
		return 1;
	}

	std::ifstream file(argv[2]);
	bool scaled = (strcmp(argv[1], "-s") ? false : true);
	if (!file.is_open()) {
		std::cerr << "Failed to open file " << argv[2] << '\n';
		return 1;
	}

	ODESystem a;
	a.readODESystem(file, scaled);
	a.simulate();

	file.close();

	return 0;
}