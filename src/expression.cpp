#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <regex>
#include <cmath>
#include <fstream>

#include "include/expression.h"
#include "include/constants.h"

void Expr::print() {
	printTree(root);
}

void Expr::printTree(Node* r) {
	if (r == nullptr) return;
	
	if (r->op == NodeType::NUM) {
		std::cout << r->value << " ";
	}
	else if (r->op == NodeType::VAR) {
		std::cout << r->name << " ";
	}
	else if (r->op == NodeType::OP || r->op == NodeType::WAVE) {
		std::cout << r->oper << " ";
	}
	else if (r->op == NodeType::INTEG) {
		std::cout << "Integrate: ";
	}

	if (r->left != nullptr) printTree(r->left);	

	if (r->right != nullptr) printTree(r->right);
}

void Expr::removeTree(Node* r) {
	if (r == nullptr) {
		return;
	}

	removeTree(r->left);
	removeTree(r->right);
	delete r;
}

double Expr::Evaluate(const std::vector<var> constants,
											const std::vector<var> vars,
											const std::vector<global_var> global) {
  
  std::vector<var> merged_vars = vars;

  for (const auto& g : global) {
  	var v;
  	v.name = g.name;
  	v.value = g.value;
  	v.rho = g.rho;
  	v.delta = g.delta;
  	merged_vars.push_back(v);
  }

  double res = 0.0;
	if (rho != 0.0) {
		if (root->op == NodeType::INTEG) {
			res = (EvaluateBUScaled(merged_vars, constants, root->right) - delta) / rho;
		}
		else {
			res = (EvaluateBUScaled(merged_vars, constants, root) - delta) / rho;
		}
	}
	else {
		if (root->op == NodeType::INTEG) {
			res = EvaluateBU(merged_vars, constants, root->right);
		}
		else {
			res = EvaluateBU(merged_vars, constants, root);
		}		
	}
	return res;
}

double Expr::EvaluateBU(const std::vector<var>& vars,
												const std::vector<var>& constants,
												Node* r) {

	double leftVal;
	double rightVal;
	if (r == nullptr) {
		return 0.0;
	}
	else if (r->op == NodeType::NUM) {
		return r->value;
	}
	else if (r->op == NodeType::VAR) {
		std::string x = r->name;
    auto it = std::find_if(constants.begin(), constants.end(), [&x](const var& c) {
        return c.name == x;
    });
		if (it != constants.end()) {
			return it->value;
		}

    it = std::find_if(vars.begin(), vars.end(), [&x](const var& v) {
        return v.name == x;
    });
		if (it != vars.end()) {
			return it->value;
		}
		else {
			throw std::invalid_argument("Variable not found\n");
		}
	}
	else if (r->op == NodeType::WAVE && r->oper == 's') {
		rightVal = EvaluateBU(vars, constants, r->right);
		return std::sin(rightVal);
	} 
	else if (r->op == NodeType::WAVE && r->oper == 'c') {
		rightVal = EvaluateBU(vars, constants, r->right);
		return std::cos(rightVal);
	}

	leftVal = EvaluateBU(vars, constants, r->left);
	rightVal = EvaluateBU(vars, constants, r->right);

	switch(r->oper) {
	case '+':
		return leftVal + rightVal;
	case '-':
		return leftVal - rightVal;
	case '*':
		return leftVal * rightVal;
	case '/':
		if (rightVal == 0.0) {
			throw std::invalid_argument("Division by 0 not possible\n");
		}
		return leftVal / rightVal;
	default:
		throw std::invalid_argument("Operation not found\n");
	}
}

double Expr::EvaluateBUScaled(const std::vector<var>& vars,
															const std::vector<var>& constants,
															Node* r) {
	double leftVal;
	double rightVal;

	if (r == nullptr) {
		return 0.0;
	}
	else if (r->op == NodeType::NUM) {
		return ((r->value / rho) + delta);
	}
	else if (r->op == NodeType::VAR) {
		std::string x = r->name;
		auto it = std::find_if(constants.begin(), constants.end(), [&x](const var& c) {
			return c.name == x;
		});
		if (it != constants.end()) {
			return ((it->value / it->rho) + it->delta);
		}

		it = std::find_if(vars.begin(), vars.end(), [&x](const var& v) {
			return v.name == x;
		});
		if (it != vars.end()) {
			return ((it->value / it->rho) + it->delta);
		}
		else {
			throw std::invalid_argument("Variable not found\n");
		}
	} 
	else if (r->op == NodeType::WAVE && r->oper == 's') {
		rightVal = EvaluateBUScaled(vars, constants, r->right);
		return std::sin(rightVal);
	} 
	else if (r->op == NodeType::WAVE && r->oper == 'c') {
		rightVal = EvaluateBUScaled(vars, constants, r->right);
		return std::cos(rightVal);
	}

	leftVal = EvaluateBUScaled(vars, constants, r->left);
	rightVal = EvaluateBUScaled(vars, constants, r->right);

	switch(r->oper) {
	case '+':
		return leftVal + rightVal;
	case '-':
		return leftVal - rightVal;
	case '*':
		return leftVal * rightVal;
	case '/':
		if (rightVal == 0.0) {
			throw std::invalid_argument("Division by 0 not possible\n");
		}
		return leftVal / rightVal;
	default:
		throw std::invalid_argument("Operation not found\n");
	}
}

void Expr::returnLeaves(Node* r, std::vector<Node*> &inp) {
	if (r == nullptr) return;
	if (r->op == NodeType::VAR || r->op == NodeType::NUM) {
		inp.push_back(r);
		return;
	}
	returnLeaves(r->left, inp);
	returnLeaves(r->right, inp);
}

void Expr::FPAAPrintConfig(std::ofstream &of, const int c,
											const std::vector<var> constants,
											const std::vector<var> vars,
											const std::vector<global_var> global,
											const std::string exprName) {
	auto inputMap = FPAASetInputs(of, c, constants);
	
	FPAASetCABs(of, root, inputMap);
	FPAASetOutputs(of, c, global, exprName);
	return;
}

std::unordered_map<std::string, std::string> Expr::FPAASetInputs(std::ofstream &of, 
																																const int c, 
																																const std::vector<var> constants) {
	std::vector<Node*> inputs;
	std::unordered_map<std::string, std::string> inputMap;

	returnLeaves(root, inputs);

	for (size_t i = 0; i < inputs.size(); i += 1) {
		std::string inputValue;
		std::string mapValue;

		auto j = std::find_if(constants.begin(), constants.end(), [&inputs, i](const var& a) {
			return a.name == inputs[i]->name;
		});
		if (j != constants.end()) {
			inputValue = std::to_string(j->value);
			mapValue = j->name;
		}
		else if (inputs[i]->op == NodeType::NUM) {
			inputValue = std::to_string(inputs[i]->value);
			mapValue = inputValue;
		}
		else {
			inputValue = inputs[i]->name;
			mapValue = inputValue;
		}
		std::string tmp = "\tFPAA" + std::to_string(c) + "_inp" + std::to_string(i);
		of << tmp << " = " << inputValue << ";\n";
		inputMap[mapValue] = tmp.substr(1);
	}
	return inputMap;
}

void Expr::FPAAPrintInputVariables(std::ofstream &of, Node* r, const std::unordered_map<std::string, std::string> inputMap) {
		if (r->op == NodeType::VAR) {
		auto tmp = inputMap.find(r->name);
		if (tmp != inputMap.end()) {
			of << tmp->second << ";\n";
		}
		else {
			throw std::invalid_argument("Variable not found\n");
		}
	}
	else if (r->op == NodeType::NUM) {
		auto tmp = inputMap.find(std::to_string(r->value));
		if (tmp != inputMap.end()) {
			of << tmp->second << ";\n";
		}
		else {
			throw std::invalid_argument("Number not found\n");
		}
	}
	else {
		of << "CAB" << r->num << ";\n";
	}
}

void Expr::FPAASetCABs(std::ofstream &of, Node* r, const std::unordered_map<std::string, std::string> inputMap) {
	if (r == nullptr || r->op == NodeType::NUM || r->op == NodeType::VAR) return;

	FPAASetCABs(of, r->left, inputMap);
	FPAASetCABs(of, r->right, inputMap);

	of << "\tCAB" << r->num << " {\n";
	switch(r->op) {
	case NodeType::INTEG:
		of << "\t\top = integ;\n\t\tinp0 = ";
		FPAAPrintInputVariables(of, r->right, inputMap);
		break;
	case NodeType::WAVE:
		switch(r->oper) {
		case 's':
			of << "\t\top = sin;\n";
      break;
		case 'c':
			of << "\t\top = cos;\n";
      break;
		default:
			throw std::invalid_argument(std::string("Invalid wave function: ") + r->oper + "\n");
		}
		of << "\t\tinp0 = ";
		FPAAPrintInputVariables(of, r->right, inputMap);
		break;
	case NodeType::OP:
		switch(r->oper) {
		case '+':
			of << "\t\top = sum;\n";
			break;
		case '-':
			of << "\t\top = min;\n";
			break;
		case '*':
			of << "\t\top = mul;\n";
			break;
		case '/':
			of << "\t\top = div;\n";
			break;
		default:
			throw std::invalid_argument("Invalid operation\n");
		}
		of << "\t\tinp0 = ";
		FPAAPrintInputVariables(of, r->left, inputMap);
		of << "\t\tinp1 = ";
		FPAAPrintInputVariables(of, r->right, inputMap);
		break;
	default:
		throw std::invalid_argument("Invalid node type\n");
	}

	of << "\t\tscale = " << rho << ";\n\t};\n";
}

void Expr::FPAASetOutputs(std::ofstream &of,
													const int c,
													const std::vector<global_var> global,
													const std::string exprName) {
	of << "\tFPAA" << c << "_outp0 = " << exprName << ";\n"; 

	auto i = global.begin();
	while (1) {
		auto n = std::find_if(i, global.end(), [&exprName](const global_var& a) {
			return a.local_name == exprName;
		});
		if (n != global.end()) {
			of << "\tFPAA" << c << "_outp1 = " << n->name << ";\n";
			i = std::next(n);
		}
		if (n == global.end()) {
			break;
		}
	}
}

bool Expr::isInteg() {
	if (root->op == NodeType::INTEG) return true;
	return false;
}

void Expr::parse(std::string e) {	
	if (e.substr(0, 5) == "integ") {
		std::regex reg(R"(\s*integ\(([^,]+)\s*,\s*([+-]?[0-9]*[.]?[0-9]+))");
		std::smatch s;
		if (regex_search(e, s, reg) && s.size() == 3) {
			tokens = tokenise(s.str(1));
			initCondit = std::stod(s.str(2));

			root = new Node(NodeType::INTEG, 0);
			root->right = buildTree(tokens);
		}
	}
	else {
		initCondit = std::stod(e);
		tokens = tokenise(e);
		root = buildTree(tokens);
	}
}

/*
*		Convert an infix tokenised vector to a tokenised vector of an expression
*		in Polish notation using the shunting yard algorithm
*/
std::vector<std::string> Expr::prefixToPolish(std::vector<std::string> v) {
	std::vector<std::string> polish;
	std::stack<std::string> s;
	for (auto t: v) {
		if (t == "+" || t == "-") {
			while (!s.empty() && (s.top() == "+" || s.top() == "-" || s.top() == "*" || s.top() == "/")) {
				polish.push_back(s.top());
				s.pop();
			}
			s.push(t);
		} 
		else if (t == "*" || t == "/") {
			while (!s.empty() && (s.top() == "*" || s.top() == "/")) {
				polish.push_back(s.top());
				s.pop();
			}
			s.push(t);
		}
		else if (t == "sin" || t == "cos" || t == "(") {
			s.push(t);
		}
		else if (t == ")") {
			while (!s.empty() && s.top() != "(") {
				polish.push_back(s.top());
				s.pop();
			}
			if (!s.empty() && s.top() == "(") {
				s.pop();
			}

			if (!s.empty() && (s.top() == "sin" || s.top() == "cos")) {
				polish.push_back(s.top());
				s.pop();
			}
		}
		else polish.push_back(t);
	}	
	while(!s.empty()) {
		polish.push_back(s.top());
		s.pop();
	}

	return polish;
}

/*
*		Tokenise an expression into a vector of strings
*/
std::vector<std::string> Expr::tokenise(const std::string e) {
	std::vector<std::string> v;
	std::regex reg(R"(sin|cos|[+-]?[0-9]*[.]?[0-9]+|[+-]?[\w]+|[*\/+\-()])");
	auto t_begin = std::sregex_iterator(e.begin(), e.end(), reg);
	auto t_end = std::sregex_iterator();

	for (std::sregex_iterator i = t_begin; i != t_end; i++) {
		std::string token = (*i).str();
		if (token[0] == '-' && (std::isalpha(token[1]) || std::isdigit(token[1]))) {
			v.push_back("-1");
			v.push_back("*");
			token = token.substr(1);
		}
		v.push_back(token);
	} 

	v = prefixToPolish(v);
	return v;
}

/*
*		Build a tree from a reverse polish notation tokenised expression
*/
Node* Expr::buildTree(std::vector<std::string>& tokens) {
	std::stack<Node*> nodeStack;
	int c = 1;
	for (auto& t: tokens) {
		if (std::isdigit(t[0]) || (t[0] == '-' && (int)t.length() > 1)) {
			nodeStack.push(new Node(std::stod(t), c));
		}
		else if (t == "sin" || t == "cos") {
			Node* n = new Node(NodeType::WAVE, t[0], c);
			n->right = nodeStack.top();
			nodeStack.pop();
			nodeStack.push(n);
		}		
		else if (std::isalpha(t[0]) || t[0] == '_') {
			nodeStack.push(new Node(t, c));
		}
		else {
			Node* n = new Node(t[0], c);
			n->right = nodeStack.top();
			nodeStack.pop();
			n->left = nodeStack.top();
			nodeStack.pop();
			nodeStack.push(n);
		}
		c += 1;
	}
	return nodeStack.top();
}

/*
*		Perform a BU walk and set the scalar for every node
*/

void Expr::setScalar(std::pair<double,double> i) {

	if (i.first != i.second) {
		rho = (2 * FPAALIM) / (i.second - i.first);
		delta = (i.first + i.second) / 2;
		initCondit = rho * (initCondit - delta); 
	}
	else {
		rho = FPAALIM / std::max(std::abs(i.first), std::abs(i.second));
		delta = 0.0;
		initCondit *= rho;

		if (root->op == NodeType::NUM) {
			root->value *= rho;
		}
	}

	std::cout << rho << ' ' << delta << '\n';
}

double Expr::getRho() {
	return rho;
}

double Expr::getDelta() {
	return delta;
}

double Expr::getInit() {
	return initCondit;
}

Node* Expr::getRoot() {
	return root;
}
