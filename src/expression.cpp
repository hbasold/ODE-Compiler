#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stack>
#include <algorithm>
#include <unordered_map>
#include <cstdlib>

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
  
  std::vector<var> merged_vars = constants;
  for (const auto& v : vars) {
    merged_vars.push_back(v);
  }	

  for (const auto& g : global) {
  	var v;
  	v.name = g.name;
  	v.value = g.value;
  	v.scalar = g.scalar;
  	merged_vars.push_back(v);
  }

  double res = 0.0;
	if (scalar != 0.0) {
		if (root->op == NodeType::INTEG) {
			res = EvaluateBUScaled(merged_vars, root->right) * scalar;
		}
		else {
			res = EvaluateBUScaled(merged_vars, root) * scalar;
		}
	}
	else {
		if (root->op == NodeType::INTEG) {
			res = EvaluateBU(merged_vars, root->right);
		}
		else {
			res = EvaluateBU(merged_vars, root);
		}		
	}
	return res;
}

double Expr::EvaluateBU(const std::vector<var>& vars,
												Node* r) {

	if (r == nullptr) return 0.0;

	if (r->op == NodeType::NUM) {
		return r->value;
	}
	if (r->op == NodeType::VAR) {
		std::string x = r->name;
    auto it = std::find_if(vars.begin(), vars.end(), [&x](const var& item) {
        return item.name == x;
    });
		if (it != vars.end()) {
			return it->value;
		}
		else {
			throw std::invalid_argument("Variable not found\n");
		}
	} 

	double leftVal = EvaluateBU(vars, r->left);
	double rightVal = EvaluateBU(vars, r->right);

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
															Node* r) {

	if (r == nullptr) {
		return 0.0;
	}

	if (r->op == NodeType::NUM) {
		return r->value * scalar;
	}
	if (r->op == NodeType::VAR) {
		std::string x = r->name;
		auto it = std::find_if(vars.begin(), vars.end(), [&x](const var& item) {
			return item.name == x;
		});
		if (it != vars.end()) {
			return (it->value / it->scalar);
		}
		else {
			throw std::invalid_argument("Variable not found\n");
		}
	} 

	double leftVal = EvaluateBUScaled(vars, r->left);
	double rightVal = EvaluateBUScaled(vars, r->right);

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

bool Expr::isInteg() {
	if (root->op == NodeType::INTEG) return true;
	return false;
}

void Expr::parse(std::string e) {	
	if (e.substr(0, 5) == "integ") {
		e = e.substr(6);
		e.pop_back();
		size_t x = e.find(',');
		initCondit = std::stod(e.substr(x + 1));

		e = e.substr(0, x);
		tokens = tokenise(e);

		root = new Node(NodeType::INTEG);
		root->right = buildTree(tokens);
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
*		Perform a BU walk and set the scalar for every node
*/

void Expr::setScalar(std::pair<double,double> i) {
	scalar = FPAALIM / std::max(std::abs(i.first), std::abs(i.second));
	initCondit *= scalar;

	if (root->op == NodeType::NUM) {
		root->value *= scalar;
	}
}

double Expr::getScalar() {
	return scalar;
}

/*
*		Tokenise an expression into a vector of strings
*/
std::vector<std::string> Expr::tokenise(const std::string e) {
	std::vector<std::string> v;
	for (int i = 0; i < (int)e.length(); i += 1) {
		std::string buf;
		while (std::isspace(e[i])) i += 1;

		if (std::isalpha(e[i])) {
			while (e[i] != ' ' && e[i] != '(' && e[i] != ')' && i < (int)e.length()) {
				buf += e[i];
				i += 1;
			}
			v.push_back(buf);
			i -= 1;
		}
		else if (e[i] == '-' && std::isalpha(e[i+1])) {
			v.push_back("-1");
			v.push_back("*");
			i += 1;
			while (e[i] != ' ' && e[i] != '(' && e[i] != ')' && i < (int)e.length()) {
				buf += e[i];
				i += 1;
			}
			v.push_back(buf);
			i -= 1;
		}
		else if (std::isdigit(e[i]) || (e[i] == '-' && std::isdigit(e[i+1]))) {
			while (e[i] != ' ' && e[i] != '(' && e[i] != ')' && i < (int)e.length()) {
				buf += e[i];
				i += 1;
			}
			v.push_back(buf);
			i -= 1;
		}
		else {
			buf += e[i];
			v.push_back(buf);
		}
	} 
	v = prefixToPolish(v);
	return v;
}

/*
*		Build a tree from a reverse polish notation tokenised expression
*/
Node* Expr::buildTree(std::vector<std::string>& tokens) {
	std::stack<Node*> nodeStack;
	for (auto& t: tokens) {
		if (std::isdigit(t[0]) || (t[0] == '-' && (int)t.length() > 1)) {
			nodeStack.push(new Node(std::stod(t)));
		}
		else if (t == "sin" || t == "cos") {
			Node* n = new Node(NodeType::WAVE, t[0]);
			n->right = nodeStack.top();
			nodeStack.pop();
			nodeStack.push(n);
		}		
		else if (std::isalpha(t[0])) {
			nodeStack.push(new Node(t));
		}
		else {
			Node* n = new Node(t[0]);
			n->right = nodeStack.top();
			nodeStack.pop();
			n->left = nodeStack.top();
			nodeStack.pop();
			nodeStack.push(n);
		}
	}
	return nodeStack.top();
}

double Expr::getInit() {
	return initCondit;
}