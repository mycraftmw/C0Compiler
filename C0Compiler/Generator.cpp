#include <iostream>
#include <algorithm>
#include <string>
#include "Generator.h"
#include "Table.h"
#include "GlobalVar.h"
#include "defines.h"

using namespace std;

ofstream Generator::out;
size_t Generator::codeLine;

Table* curTable;
vector<Table*> allTable;
map<string, string> strData;
map<string, vector<int> > tpVarNo;

map<string, string> temp2reg;

bool gdebug = false;

int cmp(pair<string, int> a, pair<string, int> b) {
	return a.second > b.second;
}

bool isTpVar(string regName) {
	if (regName == "$0") return true;
	if (regName.length() < 3)
		return false;
	else return regName[0] == '@';
}

bool isNum(string s) {
	size_t i = 0;
	if (s[0] == '-')i++;
	for (; i < s.length(); ++i)
		if (s[i]<'0' || s[i]>'9') return false;
	return true;
}

// for Temp reg simulate // can update
bool usedTreg[32];

void clrTreg() {
	for (int i = 0; i < 32; ++i)usedTreg[i] = false;
}
// clr reg directly
void clrTreg(string treg) {
	int i = stoi(treg.substr(1, treg.length() - 1));
	if (gdebug)cout << "clr $" << i << endl;
	usedTreg[i] = false;
}
// delay clean
string getTreg() {
	for (int i = 8; i < 16; i++)
		if (!usedTreg[i]) {
			usedTreg[i] = true;
			if (gdebug)cout << "return $" + to_string(i) << endl;
			return "$" + to_string(i);
		}
	if (!usedTreg[24] || !usedTreg[25]) {
		if (!usedTreg[24]) { usedTreg[24] = true; return "$24"; }
		if (!usedTreg[25]) { usedTreg[25] = true; return "$25"; }
	}
	for (map<string, string>::iterator it = temp2reg.begin();
		it != temp2reg.end();
		it++) {
		if (gdebug)cout << "nowREG -> " << it->first << ' ' << temp2reg[it->first] << endl;
	}

	for (map<string, string>::iterator it = temp2reg.begin();
		it != temp2reg.end();
		it++) {
		if (gdebug)cout << "CHECK ^^^^ " << it->first << " " << temp2reg[it->first] << endl;
		if (!(Generator::laterUse(it->first, Generator::codeLine))) {
			string r = temp2reg[it->first];
			if (gdebug)cout << Generator::codeLine + 1 << " clear " << it->first << " reuse " << r << endl;
			temp2reg.erase(it);
			return r;
		}
		else {
			if (gdebug)cout << "LUSE ---- " << it->first << " " << temp2reg[it->first] << endl;
		}
	}
	return "";
}

string tp2reg(string name) {
	if (gdebug)cout << "line:" << Generator::codeLine + 1 << " " << name << endl;
	if (name == "$0") return "$0";
	if (temp2reg.count(name)) {
		if (gdebug)cout << name << " i " << temp2reg[name] << endl;
		return temp2reg[name];
	}
	string s = getTreg();
	if (gdebug)cout << name << " o " << s << endl;
	if (s == "") {
		// get stack
		cout << "all regs are used, now should use stack!\n";
		exit(0);
	}
	else
		temp2reg[name] = s;
	//if(gdebug)cout << name << " n " << temp2reg[name] << endl;
	return temp2reg[name];
}

void Generator::saveRegs(int st, int ed) {
	int sz = ed - st;
	sz *= 4;
	out << "SUB $sp, $sp, " << sz << endl;
	for (int regN = st; regN < ed; regN++)
		out << "SW $" + to_string(regN) + ", " + to_string((regN - st) * 4) + "($sp)" << endl;
	out << "ADD $fp, $sp, " << sz << endl;
}

void Generator::restoreRegs(int st, int ed)
{
	int sz = ed - st;
	sz *= 4;
	for (int regN = st; regN < ed; regN++)
		//if (regN != 29)
		out << "LW $" + to_string(regN) + ", " + to_string((regN - st) * 4) + "($sp)" << endl;
	out << "ADD $sp, $sp, " << sz << endl;
}

int Generator::init(string filepath) {
	out.open(filepath);
	return 0;
}

Table * getCurTable(string name) {
	for (size_t i = 0; i < allTable.size(); i++) {
		if (allTable[i]->name == name)
			return allTable[i];
	}
	return nullptr;
}

int Generator::generateMips(vector<Midcode> midcodes)
{
	string op, s1, s2, d;
	Table * table = getCurTable("$");
	size_t i;

	// 临时变量引用行号统计
	tpVarNo.clear();
	for (codeLine = 0; codeLine < midcodes.size(); ++codeLine) {
		if (isTpVar(midcodes[codeLine].src1))
			tpVarNo[midcodes[codeLine].src1].push_back(codeLine);
		if (isTpVar(midcodes[codeLine].src2))
			tpVarNo[midcodes[codeLine].src2].push_back(codeLine);
		if (isTpVar(midcodes[codeLine].dst))
			tpVarNo[midcodes[codeLine].dst].push_back(codeLine);
	}
	// 引用计数优化分配s寄存器
	for (codeLine = 0; codeLine < midcodes.size(); ++codeLine) {
		op = midcodes[codeLine].op;
		if (!(op == "CONST" || op == "VAR" || op == "ARRAY")) break;
	}
	i = codeLine;
	for (; i < midcodes.size(); ++i) {
		string name = midcodes[i].op.substr(0, midcodes[i].op.length() - 1);
		table = getCurTable(name);
		table->idSReg.clear();
		table->idUsed.clear();
		table->args.clear();
		i++;
		for (; i < midcodes.size(); i++) {
			op = midcodes[i].op;
			s1 = midcodes[i].src1;
			s2 = midcodes[i].src2;
			d = midcodes[i].dst;
			if (table->isLocalVar(s1))
				table->idUsed[s1]++;
			if (table->isLocalVar(s2))
				table->idUsed[s2]++;
			if (table->isLocalVar(d))
				table->idUsed[d]++;
			if (op == "END") {
				vector<pair<string, int> > temp;
				for (map<string, int>::iterator it = table->idUsed.begin();
					it != table->idUsed.end();
					++it)
					temp.push_back(make_pair(it->first, it->second));

				sort(temp.begin(), temp.end(), cmp);
				if (gdebug)cout << "in " << name << endl;
				for (size_t i = 0; i < temp.size() && i < 8; i++) {
					if (gdebug)cout << temp[i].first << ' ' << "$s" + to_string(i) << endl;
					table->idSReg[temp[i].first] = "$s" + to_string(i);
				}
				break;
			}
		}
	}

	// for mips code

	// handle .data
	// handle global var

	out << ".data" << endl;
	table = getCurTable("$");
	for (codeLine = 0; codeLine < midcodes.size(); codeLine++) {
		op = midcodes[codeLine].op;
		s1 = midcodes[codeLine].src1;
		s2 = midcodes[codeLine].src2;
		d = midcodes[codeLine].dst;
		if (op == "CONST")
			out << s2 << " :	.word " << d << endl;
		else if (op == "VAR")
			out << s2 << " :	.space 4\n";
		else if (op == "ARRAY")
			out << s2 << " :	.space " << d << endl;
		else
			break;
		table->setStaAddr(s2, s2);
	}
	// handle string const var
	for (map<string, string>::iterator it = strData.begin();
		it != strData.end();
		it++)
		out << it->first << " :	.asciiz \"" << it->second << '"' << endl;

	// handle code
	out << ".text\n";
	out << "J main\n";
	for (; codeLine < midcodes.size(); ++codeLine) {
		//label_entry
		out << midcodes[codeLine].op << endl;;
		string funcname = midcodes[codeLine].op.substr(0, midcodes[codeLine].op.length() - 1);
		table = getCurTable(funcname);
		//init stack
		out << "SUB $sp, $sp, " << table->stacksize << endl;
		//saveRegs init fp
		if (funcname != "main") saveRegs(8, 32);
		else out << "MOVE $fp, $sp\n";
		out << "SW $ra, 0($fp)\n";

		//handle the args if it use reg
		for (size_t a = 0; a < table->items.size(); a++) {
			if (table->items[a].kind != PARAM) break;
			string arg = table->items[a].name;
			if (table->idSReg.count(arg))
				out << "LW " << table->idSReg[arg] << ", " << table->getStaAddr(arg) << endl;
		}

		for (codeLine++; midcodes[codeLine].op != "END"; ++codeLine) {
			op = midcodes[codeLine].op;
			s1 = midcodes[codeLine].src1;
			s2 = midcodes[codeLine].src2;
			d = midcodes[codeLine].dst;
			// handle branch
			if (op == "BGE" || op == "BGT" || op == "BLE" ||
				op == "BLT" || op == "BNE" || op == "BEQ") {
				if (isNum(s1)) {
					string reg = getTreg();
					out << "LI " << reg << ", " << s1 << endl;
					if (isNum(s2)) {
						out << op << " " << reg << ", " << s2 << ", " << d << endl;
					}
					else if (isTpVar(s2)) {
						string reg2 = tp2reg(s2);
						out << op << " " << reg << ", " << reg2 << ", " << d << endl;
					}
					else if (table->idSReg.count(s2))
						out << op << " " << reg << ", " << table->idSReg[s2] << ", " << d << endl;
					else {
						string reg2 = getTreg();
						out << "LW " << reg2 << ", " << table->getStaAddr(s2) << endl;
						out << op << " " << reg << ", " << reg2 << ", " << d << endl;
						clrTreg(reg2);
					}
					clrTreg(reg);
				}
				else if (isTpVar(s1)) {
					if (isNum(s2)) {
						out << op << " " << tp2reg(s1) << ", " << s2 << ", " << d << endl;
					}
					else if (isTpVar(s2)) {
						out << op << " " << tp2reg(s1) << ", " << tp2reg(s2) << ", " << d << endl;
					}
					else if (table->idSReg.count(s2))
						out << op << " " << tp2reg(s1) << ", " << table->idSReg[s2] << ", " << d << endl;
					else {
						string reg2 = getTreg();
						out << "LW " << reg2 << ", " << table->getStaAddr(s2) << endl;
						out << op << " " << tp2reg(s1) << ", " << reg2 << ", " << d << endl;
						clrTreg(reg2);
					}
				}
				else if (table->idSReg.count(s1)) {
					if (isNum(s2)) {
						out << op << " " << table->idSReg[s1] << ", " << s2 << ", " << d << endl;
					}
					else if (isTpVar(s2)) {
						out << op << " " << table->idSReg[s1] << ", " << tp2reg(s2) << ", " << d << endl;
					}
					else if (table->idSReg.count(s2))
						out << op << " " << table->idSReg[s1] << ", " << table->idSReg[s2] << ", " << d << endl;
					else {
						string reg2 = getTreg();
						out << "LW " << reg2 << ", " << table->getStaAddr(s2) << endl;
						out << op << " " << table->idSReg[s1] << ", " << reg2 << ", " << d << endl;
						clrTreg(reg2);
					}
				}
				else {
					string reg = getTreg();
					out << "LW " << reg << ", " << table->getStaAddr(s1) << endl;
					if (isNum(s2)) {
						out << op << " " << reg << ", " << s2 << ", " << d << endl;
					}
					else if (isTpVar(s2)) {
						out << op << " " << reg << ", " << tp2reg(s2) << ", " << d << endl;
					}
					else if (table->idSReg.count(s2))
						out << op << " " << reg << ", " << table->idSReg[s2] << ", " << d << endl;
					else {
						string reg2 = getTreg();
						out << "LW " << reg2 << ", " << table->getStaAddr(s2) << endl;
						out << op << " " << reg << ", " << reg2 << ", " << d << endl;
						clrTreg(reg2);
					}
					clrTreg(reg);
				}
			}
			//const var
			else if (op == "CONST") {
				string reg = getTreg();
				out << "LI " + reg + ", " + d << endl;
				out << "SW " + reg + ", " + table->getStaAddr(s2) << endl;
				clrTreg(reg);
			}
			//  stoa a 1 @t a[1] = @t
			else if (op == "STOA") {
				string reg = getTreg();
				out << "LA " + reg + ", " + table->getStaAddr(s1) << endl;
				// compute offset
				if (isNum(s2)) {
					out << "ADD " + reg + ", " + reg + ", " << stoi(s2) * 4 << endl;
				}
				else if (isTpVar(s2)) {
					string reg2 = getTreg();
					out << "SLL " + reg2 + ", " + tp2reg(s2) << ", 2\n";
					out << "ADD " + reg + ", " + reg + ", " + reg2 << endl;
					clrTreg(reg2);
				}
				else {
					string reg2 = getTreg();
					if (table->idSReg.count(s2))
						out << "MOVE " + reg2 << ", " << table->idSReg[s2] << endl;
					else
						out << "LW " + reg2 + ", " + table->getStaAddr(s2) << endl;
					out << "SLL " + reg2 + ", " + reg2 + ", 2\n";
					out << "ADD " + reg + ", " + reg + ", " + reg2 << endl;
					clrTreg(reg2);
				}
				// store d to array
				if (isNum(d)) {
					string reg2 = getTreg();
					out << "LI " + reg2 + ", " + d << endl;
					out << "SW " + reg2 + ", 0(" + reg + ")\n";
					clrTreg(reg2);
				}
				else if (isTpVar(d)) {
					out << "SW " + tp2reg(d) + ", 0(" + reg + ")\n";
				}
				else if (table->idSReg.count(d))
					out << "SW " + table->idSReg[d] << ", 0(" + reg + ")\n";
				else {
					string reg2 = getTreg();
					out << "LW " + reg2 + ", " + table->getStaAddr(d) << endl;
					out << "SW " + reg2 + ", 0(" + reg + ")\n";
					clrTreg(reg2);
				}
				clrTreg(reg);
			}
			// geta d 1 @1
			else if (op == "GETA") {
				string reg = getTreg();
				out << "LA " + reg + ", " + table->getStaAddr(s1) << endl;
				// compute offset
				if (isNum(s2)) {
					out << "ADD " + reg + ", " + reg + ", " << stoi(s2) * 4 << endl;
				}
				else if (isTpVar(s2)) {
					string reg2 = getTreg();
					out << "SLL " + reg2 + ", " + tp2reg(s2) << ", 2\n";
					out << "ADD " + reg + ", " + reg + ", " + reg2 << endl;
					clrTreg(reg2);
				}
				else {
					string reg2 = getTreg();
					if (table->idSReg.count(s2))
						out << "MOVE " + reg2 << ", " << table->idSReg[s2] << endl;
					else
						out << "LW " + reg2 + ", " + table->getStaAddr(s2) << endl;
					out << "SLL " + reg2 + ", " + reg2 + ", 2\n";
					out << "ADD " + reg + ", " + reg + ", " + reg2 << endl;
					clrTreg(reg2);
				}
				//store array to reg
				if (isTpVar(d)) {
					out << "LW " + tp2reg(d) + ", 0(" + reg + ")\n";
				}
				else if (table->idSReg.count(d))
					out << "LW " << table->idSReg[d] << ", 0(" + reg + ")\n";
				else {
					string reg2 = getTreg();
					out << "LW " + reg2 + ", 0(" + reg + ")\n";
					out << "SW " + reg2 + ", " + table->getStaAddr(d) << endl;
					clrTreg(reg2);
				}
				clrTreg(reg);
			}
			else if (op == "RETURN") {
				// handle return value
				if (s1 != "-") {
					if (isNum(s1))
						out << "LI $v0, " + s1 << endl;
					else if (isTpVar(s1))
						out << "MOVE $v0, " + tp2reg(s1) << endl;
					else if (table->idSReg.count(s1))
						out << "MOVE $v0, " + table->idSReg[s1] << endl;
					else
						out << "LW $v0, " << table->getStaAddr(s1) << endl;
				}
				// handle return
				out << "LW $ra, 0($fp)" << endl;

				if (funcname != "main")
					restoreRegs(8, 32);
				out << "ADD $sp, $sp, " << table->stacksize << endl;
				if (funcname == "main") {
					out << "LI $v0, 10" << endl;
					out << "SYSCALL" << endl;
				}
				else
					out << "JR $ra" << endl;
			}
			else if (op == "J") {
				out << "J " << s1 << endl;
			}
			else if (op[op.size() - 1] == ':') {
				out << op << endl;
			}
			else if (op == "SCANI") {
				out << "LI $v0, 5" << endl;
				out << "syscall" << endl;
				if (table->idSReg.count(s1))
					out << "MOVE " << table->idSReg[s1] << ", $v0\n";
				else
					out << "SW $v0, " << table->getStaAddr(s1) << endl;
			}
			else if (op == "SCANC") {
				out << "LI $v0, 12" << endl;
				out << "syscall" << endl;
				if (table->idSReg.count(s1))
					out << "MOVE " << table->idSReg[s1] << ", $v0\n";
				else
					out << "SW $v0, " << table->getStaAddr(s1) << endl;
			}
			else if (op == "PRTI") {
				if (isNum(s1))
					out << "LI $a0, " + s1 << endl;
				else if (isTpVar(s1))
					out << "MOVE $a0, " + tp2reg(s1) << endl;
				else if (table->idSReg.count(s1))
					out << "MOVE $a0, " << table->idSReg[s1] << endl;
				else
					out << "LW $a0, " << table->getStaAddr(s1) << endl;;

				out << "LI $v0, 1" << endl;
				out << "SYSCALL" << endl;
			}
			else if (op == "PRTC") {
				if (isNum(s1))
					out << "LI $a0, " + s1 << endl;
				else if (isTpVar(s1))
					out << "MOVE $a0, " + tp2reg(s1) << endl;
				else if (table->idSReg.count(s1))
					out << "MOVE $a0," << table->idSReg[s1] << endl;
				else
					out << "LW $a0, " + table->getStaAddr(s1) << endl;;

				out << "LI $v0, 11" << endl;
				out << "SYSCALL" << endl;
			}
			else if (op == "PRTS") {
				out << "LA $a0, " << s1 << endl;
				out << "LI $v0, 4" << endl;
				out << "SYSCALL" << endl;
			}
			else if (op == "CALL") {
				// call
				// before jump place the args
				Table * tTable = getCurTable(s1);
				int sz = tTable->stacksize;
				int as = tTable->argsnum;
				int pidx = 4;
				string reg = getTreg();
				for (int i = as - 1; i >= 0; i--) {
					out << "LW " << reg << ", " << i * 4 << "($sp)\n";
					out << "SW " << reg << ", " << pidx - sz << "($sp)\n";
					pidx += 4;
				}
				clrTreg(reg);

				out << "JAL " + s1 << endl;
				out << "ADD $sp, $sp, " << as * 4 << endl;
			}
			// ADD SUB MUL DIV
			else if (op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV") {
				string reg;
				string reg2;
				bool f1 = false, f2 = false;
				//handle s1
				if (isNum(s1)) {
					reg = getTreg();
					f1 = true;
					out << "LI " + reg + ", " + s1 << endl;
				}
				else if (isTpVar(s1))
					reg = tp2reg(s1);
				else if (table->idSReg.count(s1))
					reg = table->idSReg[s1];
				else {
					reg = getTreg();
					f1 = true;
					out << "LW " + reg + ", " + table->getStaAddr(s1) << endl;
				}
				//handle s2
				if (isNum(s2)) {
					reg2 = getTreg();
					f2 = true;
					out << "LI " + reg2 + ", " + s2 << endl;
				}
				else if (isTpVar(s2))
					reg2 = tp2reg(s2);
				else if (table->idSReg.count(s2))
					reg2 = table->idSReg[s2];
				else {
					reg2 = getTreg();
					f2 = true;
					out << "LW " + reg2 + ", " + table->getStaAddr(s2) << endl;
				}
				//handle result
				if (isTpVar(d))
					out << op << " " << tp2reg(d) << ", " << reg << ", " << reg2 << endl;
				else if (table->idSReg.count(d))
					out << op << " " << table->idSReg[d] << ", " << reg << ", " << reg2 << endl;
				else {
					string rrg = getTreg();
					out << op << " " << rrg << ", " << reg << ", " << reg2 << endl;
					out << "SW " + rrg + ", " + table->getStaAddr(d) << endl;
					clrTreg(rrg);
				}

				if (f1)clrTreg(reg);
				if (f2)clrTreg(reg2);
			}
			else if (op == "PUSH") {
				if (isNum(s1)) {
					string reg = getTreg();
					out << "LI " + reg + ", " + s1 << endl;
					out << "SUB $sp, $sp, 4" << endl;
					out << "SW " + reg + ", 0($sp)" << endl;;
					clrTreg(reg);
				}
				else if (isTpVar(s1)) {
					out << "SUB $sp, $sp, 4" << endl;
					out << "SW " + tp2reg(s1) + ", 0($sp)" << endl;;
				}
				else if (table->idSReg.count(s1)) {
					out << "SUB $sp, $sp, 4" << endl;
					out << "SW " + table->idSReg[s1] + ", 0($sp)" << endl;;
				}
				else {
					string reg = getTreg();
					out << "LW " + reg << ", " << table->getStaAddr(s1) << endl;
					out << "SUB $sp, $sp, 4" << endl;
					out << "SW " + reg + ", 0($sp)" << endl;;
					clrTreg(reg);
				}
			}
			else if (op == "ASSIGN") {
				if (s1[0] == '$')
					out << "MOVE " + tp2reg(s2) + ", $v0" << endl;
				else if (isNum(s1)) {
					if (table->idSReg.count(s2))
						out << "LI " + table->idSReg[s2] << ", " << s1 << endl;
					else {
						string reg = getTreg();
						out << "LI " + reg + ", " + s1 << endl;
						out << "SW " + reg + ", " + table->getStaAddr(s2) << endl;
						clrTreg(reg);
					}
				}
				else if (isTpVar(s1))
					if (table->idSReg.count(s2))
						out << "MOVE " + table->idSReg[s2] + ", " + tp2reg(s1) << endl;
					else
						out << "SW " + tp2reg(s1) + ", " + table->getStaAddr(s2) << endl;
				else if (table->idSReg.count(s1)) {
					if (table->idSReg.count(s2))
						out << "MOVE " + table->idSReg[s2] + ", " + table->idSReg[s1] << endl;
					else
						out << "SW " + table->idSReg[s1] + ", " + table->getStaAddr(s2) << endl;
				}
				else {
					string reg = getTreg();
					out << "LW " + reg + ", " + table->getStaAddr(s1) << endl;
					out << "SW " + reg + ", " + table->getStaAddr(s2) << endl;
					clrTreg(reg);
				}
			}
		}
		out << "LW $ra, 0($fp)\n";

		//restore regs
		if (funcname != "main")
			restoreRegs(8, 32);

		out << "ADD $sp, $sp, " << table->stacksize << endl;
		if (funcname == "main") {
			out << "LI $v0, 10\n";
			out << "SYSCALL\n";
		}
		else
			out << "JR $ra\n";
	}
	return 0;
}

bool Generator::laterUse(string tempName, int n)
{
	if (gdebug)cout << "start " << tempName << " srh:";
	if (tpVarNo[tempName].size() == 0) return true;
	for (size_t i = 0; i < tpVarNo[tempName].size(); i++) {
		if (gdebug)cout << tpVarNo[tempName][i] + 1 << " ";
		if (tpVarNo[tempName][i] >= n) return true;
	}
	if (gdebug)cout << " es" << endl;
	return false;
}