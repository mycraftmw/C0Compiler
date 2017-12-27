#ifndef GLOBALVAR_H
#define GLOBALVAR_H
#include "Table.h"
#include <map>
//#include <map>
extern Table *curTable;
extern vector<Table*> allTable;
extern map<string, string> strData;
extern map<string, vector<int> > tpVarNo;

#endif // !GLOBALVAR_H
