#include "Midcode.h"
#include "Table.h"
#include "GlobalVar.h"

Midcode::Midcode(string op, string src1, string src2, string dst)
{
	this->op = op;
	if (curTable->name != "$" && canReplace(op)) {//local const to value
		if (curTable->isConst(src1)) {
			src1 = to_string(curTable->getTitem(src1).value);
		}
		if (curTable->isConst(src2)) {
			src2 = to_string(curTable->getTitem(src2).value);
		}
	}
	this->src1 = src1;
	this->src2 = src2;
	this->dst = dst;
}

bool Midcode::canReplace(string op)
{
	return op == "ADD" || op == "SUB" || op == "MUL" || op == "DIV" || op == "STOA"
		|| op == "GETA" || op == "ASSIGN" || op == "BNE" || op == "BEQ" || op == "BLE"
		|| op == "BLT" || op == "BGE" || op == "BGT";
}

string Midcode::toString()
{
	return op + " " + src1 + " " + src2 + " " + dst;
}