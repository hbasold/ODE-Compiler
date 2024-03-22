#include <iostream>

#include "include/expression.h"

int Expr::parse (const std::string e) {
	ex = e;
	if (e.substr(0,4) == "integ") {
		t = INTEG;
	}
	else {
		t = NUM;
	}
	return 0;
}