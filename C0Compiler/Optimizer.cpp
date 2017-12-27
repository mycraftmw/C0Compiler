#include "Optimizer.h"
#include "GlobalVar.h"
#include <iostream>

ofstream Optimizer::fout;
vector<Midcode> Optimizer::midcodes;
vector<Midcode> Optimizer::optcodes;

map<string, vector<int> > idUse;

bool debug = false;

bool isTpVar(string regName);

bool isNum(string s);

bool laterUse(string tempName, int n)
{
	for (size_t i = 0; i < idUse[tempName].size(); i++)
		if (idUse[tempName][i] >= n) return true;
	return false;
}

bool isASMD(string s) {
	return s == "ADD" || s == "SUB" || s == "MUL" || s == "DIV";
}

bool isExpOp(string s) {
	return isASMD(s) || s == "ASSIGN";
}

void Optimizer::buildDAG(int begin, int end)
{
	map<string, DAGNode*> id2node;
	vector<DAGNode*> allnodes;
	vector<Midcode> tpcodes;
	DAGNode * tnode = NULL;
	string s1, s2, s3, s4;
	int handlednum = 0;
	int shareReg = 0;
	for (int i = begin; i < end; i++) {
		s1 = midcodes[i].op;
		s2 = midcodes[i].src1;
		s3 = midcodes[i].src2;
		s4 = midcodes[i].dst;
		if (s1 == "ASSIGN") { // s3 = s2;
			if (id2node[s2] == NULL) {
				id2node[s2] = new DAGNode(true, s2);
				allnodes.push_back(id2node[s2]);
			}
			if (s2 != s3) {
				if (!isTpVar(s3) && id2node[s3] != NULL && id2node[s3]->ids.size() > 1)
					id2node[s3]->rmId(s3);
				tpcodes.push_back(Midcode("ADD", id2node[s2]->ids[id2node[s2]->ids.size() - 1], "$0", s3));
				id2node[s2]->addId(s3);
				id2node[s3] = id2node[s2];
			}
		}
		else {
			if (id2node[s2] == NULL) {
				id2node[s2] = new DAGNode(true, s2);
				allnodes.push_back(id2node[s2]);
			}
			if (id2node[s3] == NULL) {
				id2node[s3] = new DAGNode(true, s3);
				allnodes.push_back(id2node[s3]);
			}
			bool isExist = false;
			for (size_t i = 0; i < allnodes.size(); i++) {
				tnode = allnodes[i];
				if (!tnode->isLeafNode && shareReg < 6 &&
					tnode->op == s1 && tnode->lson == id2node[s2] && tnode->rson == id2node[s3]) {
					if (tnode->ids.size() == 1) shareReg++;
					tpcodes.push_back(Midcode("ADD", tnode->ids[tnode->ids.size() - 1], "$0", s4));
					tnode->addId(s4);
					id2node[s4] = tnode;
					isExist = true;
				}
			}
			if (!isExist) {
				DAGNode * node = new DAGNode(false, s1);
				allnodes.push_back(node);
				node->addId(s4);
				node->lson = id2node[s2];
				node->rson = id2node[s3];
				id2node[s2]->fn++;
				id2node[s3]->fn++;
				id2node[s4] = node;
			}
		}
	}

	reverse(tpcodes.begin(), tpcodes.end());

	while (1) {
		for (int i = allnodes.size() - 1; i >= 0; i--) {
			tnode = allnodes[i];
			if (tnode->fn == 0 && !tnode->isHandle) {
				if (debug)cout << "now node " << tnode->op << " " << tnode->ids[0] << " ---- ";
				/*for (size_t i = 1; i < tnode->ids.size(); i++) {
					if (debug)cout << tnode->ids[i] << ' ';
					if (!isTpVar(tnode->ids[i]) || laterUse(tnode->ids[i], end)) {
						tpcodes.push_back(Midcode("ASSIGN", tnode->ids[0], tnode->ids[i], "-"));
					}
				}*/

				if (tnode->ids.size() && tnode->lson != NULL) {
					string lop = tnode->lson->ids[0];
					string rop = tnode->rson->ids[0];
					string res = tnode->ids[0];
					tpcodes.push_back(Midcode(tnode->op, lop, rop, res));
					tnode->lson->fn--;
					tnode->rson->fn--;
				}
				tnode->isHandle = true;
				handlednum++;
				if (debug)cout << "over" << endl;
			}
		}
		if (handlednum == allnodes.size()) {
			for (int i = tpcodes.size() - 1; i >= 0; i--)
				optcodes.push_back(tpcodes[i]);
			break;
		}
	}
	// clean
	for (size_t i = 0; i < allnodes.size(); i++) {
		delete allnodes[i];
	}
	allnodes.clear();
}

void Optimizer::init(string filename)
{
	fout.open(filename);
}

void Optimizer::optimize(vector<Midcode> initMidcodes)
{
	midcodes = initMidcodes;
	for (size_t codeLine = 0; codeLine < midcodes.size(); ++codeLine) {
		if (isTpVar(midcodes[codeLine].src1))
			idUse[midcodes[codeLine].src1].push_back(codeLine);
		else if (isTpVar(midcodes[codeLine].src2))
			idUse[midcodes[codeLine].src2].push_back(codeLine);
		else if (isTpVar(midcodes[codeLine].dst))
			idUse[midcodes[codeLine].dst].push_back(codeLine);
	}

	// dag
	dagOptimize();
	// 常量优化
	constOptimize();
	// dataflow
	// color
}

void Optimizer::outputOptcode()
{
	for (size_t i = 0; i < optcodes.size(); i++) {
		fout << optcodes[i].toString() << endl;
	}
}

void Optimizer::constOptimize()
{
	for (size_t codeLine = 0; codeLine < optcodes.size(); ++codeLine) {
		string op = optcodes[codeLine].op;
		string s1 = optcodes[codeLine].src1;
		string s2 = optcodes[codeLine].src2;
		if (isASMD(op) && isNum(s1) && isNum(s2)) {
			int k1 = stoi(s1);
			int k2 = stoi(s2);
			int ans = 0;
			if (op == "ADD")ans = k1 + k2;
			else if (op == "SUB")ans = k1 - k2;
			else if (op == "MUL")ans = k1 * k2;
			else if (op == "DIV")ans = k1 / k2;
			optcodes[codeLine].op = "ADD";
			optcodes[codeLine].src1 = "$0";
			optcodes[codeLine].src2 = to_string(ans);
		}
	}
}

void Optimizer::dagOptimize()
{
	int begin, end;
	for (size_t i = 0; i < midcodes.size(); i++) {
		if (isExpOp(midcodes[i].op) &&
			!(midcodes[i].op == "ASSIGN" && (midcodes[i].src1[0] == '$' || isNum(midcodes[i].src1)))) {
			begin = i;
			end = i + 1;
			while (end < (int)midcodes.size() &&
				isExpOp(midcodes[end].op) &&
				!(midcodes[end].op == "ASSIGN" && (midcodes[end].src1[0] == '$' || isNum(midcodes[end].src1))))
				end++;
			buildDAG(begin, end);
			i = end - 1;
			if (debug)cout << begin + 1 << " " << end + 1 << endl;
		}
		else
			optcodes.push_back(midcodes[i]);
	}
}

DAGNode::DAGNode(bool isLeafNode, string s)
{
	if (debug)cout << "New Node " << isLeafNode << " " << s << endl;
	this->isLeafNode = isLeafNode;
	if (isLeafNode)
		this->ids.push_back(s);
	else
		this->op = s;
	this->lson = this->rson = NULL;
	this->fn = 0;
	this->isHandle = false;
}

void DAGNode::addId(string id)
{
	if (debug) cout << "add id " + id << endl;
	if (find(ids.begin(), ids.end(), id) == ids.end())
		ids.push_back(id);
}

void DAGNode::rmId(string id)
{
	for (vector<string>::iterator it = ids.begin(); it != ids.end(); it++)
		if (*it == id) {
			ids.erase(it);
			return;
		}
}

string DAGNode::repid()
{
	for (size_t i = 0; i < ids.size(); i++)
		if (ids[i][0] != '@' || ids[i][1] == '$')
			return ids[i];
	return ids[0];
}