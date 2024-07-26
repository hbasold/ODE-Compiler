#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <fstream>
#include <cmath>
#include <unordered_map>
#include <algorithm>
#include <limits>

#include "include/odeSystem.h"

int ODESystem::editTreeDistance(const Node* root1, const Node* root2) {
	if (root1 == nullptr) {
		if (root2) {
			return 1 + editTreeDistance(nullptr, root2->left) + editTreeDistance(nullptr, root2->right);
		}
		return 0;
	}
	if (root2 == nullptr) {
		if (root1) {
			return 1 + editTreeDistance(root1->left, nullptr) + editTreeDistance(root1->right, nullptr);
		}
		return 0;
	}

	int init = (root1->op == root2->op && root1->oper == root2->oper) ? 0 : 1;
	
	std::vector<int> minElement = {
		1 + editTreeDistance(root1, root2->left),
		1 + editTreeDistance(root1, root2->right),
		1 + editTreeDistance(root1->left, root2),
		1 + editTreeDistance(root1->right, root2),
		init + editTreeDistance(root1->left, root2->left) + editTreeDistance(root1->right, root2->right)
	};
	std::vector<int>::iterator res = std::min_element(minElement.begin(), minElement.end());

	return *res;
}

std::vector<std::vector<int>> ODESystem::computeSimilarityMatrix(const std::vector<Expr*> vars) {
	std::vector<std::vector<int>> matrix(vars.size(), std::vector<int>(vars.size(), 0));

	int distance;
	for (size_t i = 0; i < vars.size(); i += 1) {
		for (size_t j = i + 1; j < vars.size(); j += 1) {
			distance = editTreeDistance(vars[i]->getRoot(), vars[j]->getRoot());
			matrix[i][j] = distance;
			matrix[j][i] = distance;
		}
	}

	return matrix;
}

std::pair<int, int> findMinPair(const std::vector<std::vector<int>> simMatrix) {
	int minSim = std::numeric_limits<int>::max();
	std::pair<int, int> minPair;

	for (size_t i = 0; i < simMatrix.size(); i += 1) {
		for (size_t j = 0; j < simMatrix[i].size(); j += 1) {
			if (i != j && simMatrix[i][j] < minSim) {
				minSim = simMatrix[i][j];
				minPair = {i, j};
			}
		}
	}
	return minPair;
}

ODE ODESystem::cluster(ODE ode) {
	std::vector<std::vector<int>> simMatrix = computeSimilarityMatrix(ode.varValues);

	size_t n = ode.varValues.size();

	std::vector<int> clusterLabels(n);
	for (size_t i = 0; i < n; i += 1) {
		clusterLabels[i] = i;
	}

	int cur = n;
	while (cur > 1) {
		std::pair<int, int> minPair = findMinPair(simMatrix);
		int oldCur = clusterLabels[minPair.second];
		int newCur = clusterLabels[minPair.first];
		for (size_t i = 0; i < n; i += 1) {
			if (clusterLabels[i] == oldCur) clusterLabels[i] = newCur;
		}

		for (size_t i = 0; i < n; i += 1) {
			if (clusterLabels[i] == newCur) {
				for (size_t j = 0; j < n; j += 1) {
					if (clusterLabels[j] != newCur) {
						simMatrix[i][j] = std::min(simMatrix[i][j], simMatrix[minPair.second][j]);
						simMatrix[j][i] = simMatrix[i][j];
					}
				}
			}
		}
		cur -= 1;
	}

	std::unordered_map<int, std::vector<Expr*>> clusters;
	std::unordered_map<int, std::vector<std::string>> clusterVarNames;
	std::unordered_map<int, std::vector<std::pair<double, double>>> clusterInterval;
	for (size_t i = 0; i < n; i += 1) {
		clusters[clusterLabels[i]].push_back(ode.varValues[i]);
		clusterVarNames[clusterLabels[i]].push_back(ode.varNames[i]);
		clusterInterval[clusterLabels[i]].push_back(ode.interval[i]);
	}

	std::vector<Expr*> reorderedExpr;
	std::vector<std::string> reorderedNames;
	std::vector<std::pair<double, double>> reorderedInterval;
	for (auto& cl : clusters) {
		for (auto e : cl.second) {
			reorderedExpr.push_back(e);
		}
	}
	for (auto& cl : clusterVarNames) {
		for (auto name : cl.second) {
			reorderedNames.push_back(name);
		}
	}
	for (auto& cl : clusterInterval) {
		for (auto interval : cl.second) {
			reorderedInterval.push_back(interval);
		}
	}

	ODE ret;
	ret.varNames = reorderedNames;
	ret.varValues = reorderedExpr;
	ret.interval = reorderedInterval;
	ret.time = ode.time;

	return ret;
}