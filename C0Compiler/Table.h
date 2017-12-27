#ifndef TABLE_H
#define TABLE_H
#include <vector>
#include <map>
#include "Titem.h"

using namespace std;

class Titem;
class Table
{
public:
	string name;// table name
	Table *fTable;// father table
	vector<Titem> items;// items for table
	int rtype;// reutrn type
	int stacksize;// stack size
	int argsnum;// args num

	map<string, int> idUsed;// ref count
	map<string, string> idSReg;// id to SReg
	vector<string> args;//for func args

	Table(string name);// new named table
	Table(Table * fTable);// new son table

	int func_ins(int type, string name, Table* funcTable);
	int const_ins(int type, string name, int value);
	int var_ins(int type, string name, int length);
	int arg_ins(int type, string name);

	bool named(string name);
	bool isConst(string name);
	bool isVar(string name);
	bool isLocalVar(string name);
	bool isFunc(string name);
	bool isArray(string name);
	int getType(string name);
	string getStaAddr(string name);
	int setStaAddr(string name, string addr);
	Titem getTitem(string name);
};

#endif // !TABLE_H
