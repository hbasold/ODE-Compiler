/******************************************************************************\
*Header file for the expression class
*Expressions are parsed into an abstract syntax tree
*
\******************************************************************************/
#ifndef EXPRH
#define EXPRH

#include <string>
#include <vector>
#include <unordered_map>

struct var {
	std::string name;
	double value;
	double scalar;
};

struct global_var {
	std::string local_name;
	std::string name;
	double value;
	double scalar;
};

enum class NodeType {
	NUM,
	VAR,
	INTEG,
	WAVE,
	OP,
};

struct Node {
	NodeType op;

	double value;					//if the node is a num
	std::string name;				//if the node is a variable
	char oper;

	Node* left;
	Node* right;

	//Constructors for the various node types
	Node(NodeType o) : op(o), left(nullptr), right(nullptr) {}
	Node(char c) : op(NodeType::OP), oper(c), left(nullptr), right(nullptr) {}
	Node(NodeType o, char c) : op(o), oper(c), left(nullptr), right(nullptr) {}
	Node(double v) : op(NodeType::NUM), value(v), left(nullptr), right(nullptr) {}
	Node(std::string n) : op(NodeType::VAR), name(n), left(nullptr), right(nullptr) {}
};

class Expr {
public:
	Expr() : root(nullptr), initCondit(0.0), scalar(0.0){}
	~Expr() {
		removeTree(root);
	}

	void print();

	void parse(std::string e);

	double getInit();

	double Evaluate(const std::vector<var> constants,
					const std::vector<var> vars,
					const std::vector<global_var> global);

	bool isInteg();

	void setScalar(std::pair<double,double> i);
	double getScalar();

private:
	Node* parseTree(std::string e);
	Node* root;
	std::vector<std::string> tokenise(const std::string e);
	std::vector<std::string> prefixToPolish(std::vector<std::string> v);
	Node* buildTree(std::vector<std::string>& tokens);

	void removeTree(Node* r);

	void printTree(Node* r);

	double EvaluateBU(const std::vector<var>& vars, Node* r);
	double EvaluateBUScaled(const std::vector<var>& vars, Node* r);

	std::vector<std::string> tokens{};
	double initCondit;
	double scalar;
};

#endif