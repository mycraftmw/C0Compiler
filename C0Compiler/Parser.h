#ifndef PARSER_H
#define PARSER_H
#include <vector>
#include <fstream>
#include "Midcode.h"
#include "Table.h"

using namespace std;

class Parser
{
private:
	static int label_n;
	static int tpreg_n;
	static ofstream fout;
	static string newLabel();
	static string newName();

public:
	static int error_num;
	static vector<Midcode> midcodes;

	static void init(string filename);
	static void outputMidcode();

	static int program();              //主程序
	static int const_declaration();        //常量说明
	static int var_declaration();
	static int var_func_declaration();
	static int func_declaration(int type, string name);

	static int statement();
	static int condition(string suc, string fail);
	static int expression(int&type, string&res);
	static int term(int&type, string&res);
	static int factor(int&type, string&res);
	static int callFunc(string name, int&type, string&res);
	static int argsPass(string name, int&argNum);

	static int ttp2tp(int&type);
	static int error(int error_no);
	static int inscode(string s1, string s2, string s3, string s4);
	static int skipTo(int count, ...);
};

#endif // !PARSER_H
