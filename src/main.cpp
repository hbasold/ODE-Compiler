#include <iostream>
#include <fstream>

#include "include/odeSystem.h"

int main(int argc, char* argv[]) {

	if (argc != 2) {
		std::cerr << "Compiler usage: " << argv[0] << " <filename.ode>\n";
		return 1;
	}

	std::ifstream file(argv[1]);
	if (!file.is_open()) {
		std::cerr << "Failed to open file " << argv[1] << '\n';
		return 1;
	}

	ODESystem a;

	a.readODESystem(file);

	file.close();

	return 0;
}