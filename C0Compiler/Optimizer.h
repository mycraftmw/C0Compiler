#ifndef OPTIMIZER_H
#define OPTIMIZER_H
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include "Midcode.h"

using namespace std;

class Optimizer
{
public:
	static vector<Midcode> midcodes;
	static vector<Midcode> optcodes;

	static void init(string filename);
	static void optimize(vector<Midcode> initMidcodes);
	static void outputOptcode();

	static void constOptimize();
	static void dagOptimize();

private:
	static ofstream fout;
	// for dag
	static vector<int> enter;
	static map<string, int> label;

	static void buildDAG(int begin, int end);
};

class DAGNode
{
public:
	bool isLeafNode;
	DAGNode *lson, *rson;
	string op;
	vector<string> ids;
	int fn;
	bool isHandle;

	DAGNode(bool isLeafNode, string s);
	void addId(string id);
	void rmId(string id);
	string repid();
};

#endif // !OPTIMIZER_H
