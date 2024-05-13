#include <iostream>
#include <fstream>

#include <getopt.h>

#include "include/odeSystem.h"

static void
showHelp(const char *progName)
{
  std::cerr << progName << " {-n|-s} [filename ...]\n";
  std::cerr <<
R"HERE(
    -n           No scaling performed.
    -s           Scale variables according to defined FPAALIM in constants.h.

    One of -n or -s must be specified.
    filename must be one file.
)HERE";
}

int main(int argc, char* argv[]) {

  const char *progName = argv[0];
  char c;

  bool noScaling = 0;
  bool scaling = 0;
  std::string inpFile;

  while ((c = getopt(argc, argv, "sni:")) != -1) {
  	switch(c) {
  	case 's':
  		scaling = 1;
  		break;
  	case 'n':
  		noScaling = 1;
			break;
	  case 'h':
    default:
      showHelp(progName);
      return -1;
  	}
  }

  if (optind < argc) {
      inpFile = argv[optind];
      optind++;
  }

  if (noScaling && scaling) {
  	std::cerr << "Error: can't use both scaling and no scaling\n";
  	return -1;
  }
  else if (!noScaling && !scaling) {
  	std::cerr << "Error: has to use either scaling or no scaling\n";
  	return -1;
  }
	std::ifstream file(inpFile);

	if (!file.is_open()) {
		std::cerr << "Error: failed to open file " << inpFile << '\n';
		return -1;
	}

	ODESystem a;
	a.readODESystem(file, scaling);
	a.simulate();

	file.close();

	return 0;
}