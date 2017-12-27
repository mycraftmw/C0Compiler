#include "defines.h"
#include "GlobalVar.h"
#include "Table.h"
#include <iostream>

Table::Table(string name)//for global
{
	this->name = name;
	fTable = NULL;
	stacksize = 0;//no use because data stored in .data
	argsnum = 0;
}

Table::Table(Table * fTable)//for func
{
	this->fTable = fTable;
	stacksize = 4;
	argsnum = 0;
}

int Table::func_ins(int type, string name, Table * funcTable)
{
	if (name != "main"&&named(name)) return -1;
	items.push_back(Titem(FUNC, type, name, funcTable));
	return 0;
}

int Table::const_ins(int type, string name, int value)
{
	if (named(name)) return -1;
	items.push_back(Titem(CONST, type, name, value, stacksize));
	stacksize += 4;
	return 0;
}

int Table::var_ins(int type, string name, int length)
{
	if (named(name)) return -1;
	if (length) {
		items.push_back(Titem(ARRAY, type, name, length, stacksize));
		stacksize += 4 * length;
	}
	else {
		items.push_back(Titem(VAR, type, name, length, stacksize));
		stacksize += 4;
	}
	return 0;
}

int Table::arg_ins(int type, string name)
{
	if (named(name))return -1;
	items.push_back(Titem(PARAM, type, name, 0, stacksize));
	stacksize += 4;
	argsnum++;
	return 0;
}

bool Table::named(string name)
{
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return true;
	return false;
}

bool Table::isConst(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].kind == CONST;
	if (fTable)
		return fTable->isConst(name);
	return false;
}

bool Table::isVar(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].kind == VAR || items[i].kind == PARAM;
	if (fTable)
		return fTable->isVar(name);
	return false;
}

bool Table::isLocalVar(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].kind == VAR || items[i].kind == PARAM;
	return false;
}

bool Table::isFunc(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].kind == FUNC;
	if (fTable)
		return fTable->isFunc(name);
	return false;
}

bool Table::isArray(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].kind == ARRAY;
	if (fTable)
		return fTable->isArray(name);
	return false;
}

int Table::getType(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].type;
	if (fTable)
		return fTable->getType(name);
	return -1;
}

string Table::getStaAddr(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i].address;
	if (fTable)
		return fTable->getStaAddr(name);
	return string();
}

int Table::setStaAddr(string name, string addr) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name) {
			items[i].address = addr;
			return 0;
		}
	if (fTable)
		return fTable->setStaAddr(name, addr);
	return -1;
}

Titem Table::getTitem(string name) {
	for (size_t i = 0; i < items.size(); i++)
		if (items[i].name == name)
			return items[i];
	if (fTable)
		return fTable->getTitem(name);
	return Titem();
}