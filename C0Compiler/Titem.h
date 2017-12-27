#ifndef TITEM_H
#define TITEM_H
#include <string>

using namespace std;

class Table;
class Titem
{
public:
	Table *fTable;
	Table *funcTable;
	string name;
	int kind; // const var array function param
	int type; // int char void
	int value; // for array is length, for func is args_num
	string sym;
	string address;

	Titem();
	Titem(int kind, int type, string name, int value, int address);// for const or var
	Titem(int kind, int type, string name, Table* funcTable);// for func
};

#endif // !TITEM_H
