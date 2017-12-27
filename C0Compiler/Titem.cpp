#include "Titem.h"
#include "Table.h"
#include "GlobalVar.h"
#include <iostream>
Titem::Titem()
{
	this->fTable = nullptr;
	this->name = "";
}
Titem::Titem(int kind, int type, string name, int value, int address)
{
	this->kind = kind;
	this->type = type;
	this->name = name;
	this->value = value;
	this->address = to_string(address) + "($fp)";
	this->fTable = curTable;
}

Titem::Titem(int kind, int type, string name, Table * funcTable)
{
	this->kind = kind;
	this->type = type;
	this->name = name;
	this->funcTable = funcTable;
	this->address = "";
}