#ifndef EXPRH
#define EXPRH

enum exprType {
	NUM,
	INTEG,
};

class Expr {
public:
	int parse (const std::string e);
private:
	exprType t;
	std::string ex;
};

#endif