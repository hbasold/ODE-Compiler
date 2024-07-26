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
	double rho;
	double delta;
};

struct global_var {
	std::string local_name;
	std::string name;
	double value;
	double rho;
	double delta;
};

enum class NodeType {
	NUM,
	VAR,
	INTEG,
	WAVE,
	OP,
};

struct Node {
	int num;
	NodeType op;

	double value;					//if the node is a num
	std::string name;				//if the node is a variable
	char oper;

	Node* left;
	Node* right;

	//Constructors for the various node types
	Node(NodeType o, int n) : num(n), op(o), left(nullptr), right(nullptr) {}
	Node(char c, int n) : num(n), op(NodeType::OP), oper(c), left(nullptr), right(nullptr) {}
	Node(NodeType o, char c, int n) : num(n), op(o), oper(c), left(nullptr), right(nullptr) {}
	Node(double v, int n) : num(n), op(NodeType::NUM), value(v), left(nullptr), right(nullptr) {}
	Node(std::string n, int c) : num(c), op(NodeType::VAR), name(n), left(nullptr), right(nullptr) {}
};

class Expr {
public:
	Expr() : root(nullptr), initCondit(0.0), rho(0.0), delta(0.0){}
	~Expr() {
		removeTree(root);
	}

	void print();

	void parse(std::string e);

	double Evaluate(const std::vector<var> constants,
					const std::vector<var> vars,
					const std::vector<global_var> global);

	bool isInteg();

	void setScalar(std::pair<double,double> i);
	
	void FPAAPrintConfig(std::ofstream &of, 
						 const int c, const std::vector<var> constants,
						 const std::vector<var> vars,
						 const std::vector<global_var> global,
						 const std::string exprName);

	double getInit();
	double getRho();
	double getDelta();
	Node* getRoot();

private:
	Node* parseTree(std::string e);
	Node* root;
	std::vector<std::string> tokenise(const std::string e);
	std::vector<std::string> prefixToPolish(std::vector<std::string> v);
	Node* buildTree(std::vector<std::string>& tokens);

	void removeTree(Node* r);

	void printTree(Node* r);

	double EvaluateBU(const std::vector<var>& vars, const std::vector<var>& constants, Node* r);
	double EvaluateBUScaled(const std::vector<var>& vars, const std::vector<var>& constants, Node* r);

	std::unordered_map<std::string, std::string> FPAASetInputs(std::ofstream &of, 
										 																				 const int c, 
										 																				 const std::vector<var> constants);
	void FPAASetOutputs(std::ofstream &of,
											const int c,
											const std::vector<global_var> global,
											const std::string exprName);
	void FPAASetCABs(std::ofstream &of, 
									 Node* r,
									 const std::unordered_map<std::string, std::string> inputMap);
	void returnLeaves(Node* r, std::vector<Node*> &inp);
	void FPAAPrintInputVariables(std::ofstream &of, 
															 Node* r, 
															 const std::unordered_map<std::string, std::string> inputMap);

	std::vector<std::string> tokens{};
	double initCondit;
	double rho;
	double delta;
};

#endif