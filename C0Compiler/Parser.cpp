#include <iostream>
#include <stdarg.h>
#include "defines.h"
#include "GlobalVar.h"
#include "Lexer.h"
#include "Parser.h"
#include "Table.h"
#include "Midcode.h"

int Parser::label_n = 0;
int Parser::tpreg_n = 0;
int Parser::error_num = 0;
ofstream Parser::fout;

vector<Midcode> Parser::midcodes;

bool PARSER_CHECK = false;

string Parser::newLabel()
{
	return "l.b." + to_string(label_n++);
}

string Parser::newName()
{
	return "@t" + to_string(tpreg_n++);
}

void Parser::init(string filename)
{
	fout.open(filename);
}

void Parser::outputMidcode()
{
	for (size_t i = 0; i < midcodes.size(); i++) {
		fout << midcodes[i].toString() << endl;
	}
}

int Parser::program()
{
	Lexer::next();

	while (Lexer::token == Const) const_declaration();
	while (Lexer::isType() || Lexer::token == Void)
		var_func_declaration();
	if (Lexer::next() > 0)
		error(1);
	if (PARSER_CHECK)
		cout << "program\n";
	/*cout << "<<< output the midcode:\n";
	for each (Midcode var in midcodes)
		cout << var.toString() << endl;*/
	return 0;
}

int Parser::const_declaration()
{
	Lexer::match(Const);
	if (Lexer::token == Int) {
		Lexer::match(Int);
		do {
			if (Lexer::ch == EOF) return -1;
			string name = Lexer::sym;
			if (Lexer::match(Id)) error_num++;
			if (Lexer::match(Assign)) error_num++;
			int x = 1;
			if (Lexer::token == Add || Lexer::token == Sub) {
				x = Lexer::token == Add ? 1 : -1;
				Lexer::match(Lexer::token);
			}
			int v = Lexer::ivalue;
			v *= x;
			if (Lexer::match(Num)) error_num++;
			// insert table
			if (curTable->const_ins(INT, name, v)) {
				error(4);
				Lexer::match(Semicolon);
				return -1;
			}
			inscode("CONST", "INT", name, to_string(v));
			if (Lexer::token == Semicolon) {
				Lexer::match(Semicolon);
				break;
			}
			if (Lexer::match(Comma)) {
				error(18);
				Lexer::match(Semicolon);
				break;
			}
		} while (true);
	}
	else if (Lexer::token == Char) {
		Lexer::match(Char);
		do
		{
			if (Lexer::ch == EOF) return -1;
			string name = Lexer::sym;
			if (Lexer::match(Id)) error_num++;
			if (Lexer::match(Assign)) error_num++;
			int v = Lexer::ivalue;
			Lexer::match(Asc);
			// insert table
			if (curTable->const_ins(CHAR, name, v)) error(4);
			inscode("CONST", "CHAR", name, to_string(v));

			if (Lexer::token == Semicolon) {
				Lexer::match(Semicolon);
				break;
			}
			if (Lexer::match(Comma)) {
				error(18);
				Lexer::match(Semicolon);
				break;
			}
		} while (true);
	}
	if (PARSER_CHECK)
		cout << "const def\n";
	return 0;
}

int Parser::var_declaration()
{
	int type = Lexer::token;
	ttp2tp(type);
	Lexer::match(Lexer::token);
	do {
		if (Lexer::ch == EOF) return -1;
		string name = Lexer::sym;
		int len = 0;
		Lexer::match(Id);
		if (Lexer::token == Lbra) {
			Lexer::match(Lbra);
			len = Lexer::ivalue;
			if (Lexer::match(Num)) error_num++;
			if (Lexer::match(Rbra)) error_num++;
		}
		else len = 0;

		// insert table
		if (curTable->var_ins(type, name, len)) error(4);
		if (len)
			inscode("ARRAY", type_words[type], name, to_string(len * 4));
		else
			inscode("VAR", type_words[type], name, "-");

		if (Lexer::token == Semicolon) {
			Lexer::match(Semicolon);
			break;
		}
		if (Lexer::match(Comma)) {
			error(2);
			Lexer::match(Semicolon);
			break;
		}
	} while (true);
	if (PARSER_CHECK)
		cout << "var def\n";
	return 0;
}

int Parser::var_func_declaration()
{
	int type = Lexer::token;
	ttp2tp(type);
	string name = "";
	int len = 0;
	if (Lexer::isType()) {
		Lexer::match(Lexer::token);
		name = Lexer::sym;
		Lexer::match(Id);
		if (Lexer::token == Lpar) { // funtion
			func_declaration(type, name);
		}
		else { // variable
			while (true) {
				if (Lexer::ch == EOF) return -1;
				if (Lexer::token == Lbra) { // array
					Lexer::match(Lbra);
					len = Lexer::ivalue;
					if (Lexer::match(Num)) error_num++;
					if (Lexer::match(Rbra)) error_num++;
				}
				else len = 0;
				// insert table
				if (curTable->var_ins(type, name, len)) error(4);
				if (len) {
					inscode("ARRAY", type_words[type], name, to_string(len * 4));
				}
				else {
					inscode("VAR", type_words[type], name, "-");
				}
				if (Lexer::token == Semicolon) {
					Lexer::match(Semicolon);
					break;
				}
				if (Lexer::match(Comma)) {
					error(2);
					Lexer::match(Semicolon);
					break;
				}
				name = Lexer::sym;
				Lexer::match(Id);
			}
		}
	}
	else if (Lexer::token == Void) {
		Lexer::match(Void);
		name = Lexer::sym;
		if (Lexer::token != Main) { // void funtion
			Lexer::match(Id);
			func_declaration(type, name);
		}
		else {
			Lexer::match(Main);
			func_declaration(type, "main");
		}
	}

	if (PARSER_CHECK)
		cout << "var_func def\n";
	return 0;
}

int Parser::func_declaration(int type, string name)
{
	// new table
	Table *newTable = new Table(curTable);
	newTable->name = name;
	newTable->rtype = type;
	if (curTable->func_ins(type, name, newTable)) {
		delete newTable;
		error(4);
		while (Lexer::token != Rcur) {
			if (Lexer::ch == EOF) return -1;
			Lexer::next();
		}
		Lexer::match(Rcur);
		return -1;
	}
	// handle args
	int param_n = 0;
	if (Lexer::match(Lpar)) error_num++;
	while (Lexer::token != Rpar) {
		int type = Lexer::token;
		ttp2tp(type);
		Lexer::match(Lexer::token);
		string name = Lexer::sym;
		Lexer::match(Id);
		// insert new table
		if (newTable->arg_ins(type, name)) error(4);
		param_n++;
		if (Lexer::token == Rpar) break;
		if (Lexer::match(Comma)) {
			error(19);
			skipTo(1, Rpar);
			break;
		}
	}
	Lexer::match(Rpar);
	curTable->items.back().value = param_n;
	// set cur to new
	// insert to allTable
	curTable = newTable;
	allTable.push_back(curTable);
	inscode(name + ":", "-", "-", "-");

	if (Lexer::match(Lcur)) error_num++;

	while (Lexer::token == Const) {
		if (Lexer::ch == EOF) return-1;
		const_declaration();
	}
	while (Lexer::isType()) {
		if (Lexer::ch == EOF) return-1;
		var_declaration();
	}
	while (Lexer::token != Rcur) {
		if (Lexer::ch == EOF) return-1;
		statement();
	}
	inscode("END", "-", "-", "-");
	//set cur to father
	curTable = curTable->fTable;

	Lexer::match(Rcur);

	if (PARSER_CHECK)
		cout << "func_def\n";
	return 0;
}

int Parser::statement()
{
	// there are 9 kinds of statements here:
	// 1. if (...) <statement> [else <statement>]
	// 2. do <statement> while(...)
	// 3. for (;;) <statement>
	// 4. { <statement> }
	// 5. return xxx;
	// 6. <empty statement>;
	// 7. expression; (expression end with semicolon including functions.)
	// 8. scanf
	// 9. printf
	if (Lexer::token == If) {
		// if (...) <statement> [else <statement>]
		//
		//   if (...)           <cond>
		//                      JZ a
		//     <statement>      <statement>
		//   else:              JMP b
		// a:
		//     <statement>      <statement>
		// b:                   b:
		string lb1 = newLabel();

		Lexer::match(If);
		if (Lexer::match(Lpar)) error_num++;
		condition("", lb1);
		if (Lexer::match(Rpar)) error_num++;
		statement();
		if (Lexer::token == Else) {
			Lexer::match(Else);
			string lb2 = newLabel();
			inscode("J", lb2, "-", "-");
			inscode(lb1 + ":", "-", "-", "-");
			statement();
			inscode(lb2 + ":", "-", "-", "-");
			if (PARSER_CHECK)
				cout << "Else \n";
		}
		else {
			inscode(lb1 + ":", "-", "-", "-");
		}
		if (PARSER_CHECK)
			cout << "If \n";
	}
	else if (Lexer::token == Do) {
		// a:                     a:
		//    do                 nop
		//      <statement>      <statement>
		//    while (<cond>)     <cond>
		//                          JZ a
		// b:                     b:
		string lb1 = newLabel();
		inscode(lb1 + ":", "-", "-", "-");
		Lexer::match(Do);
		statement();
		if (Lexer::match(While)) error_num++;
		if (Lexer::match(Lpar)) error_num++;
		condition(lb1, "");
		if (Lexer::match(Rpar)) error_num++;

		if (PARSER_CHECK)
			cout << "Do \n";
	}
	else if (Lexer::token == For) {
		//   for(i=1;a: i<10 J b or c ; d: i=i+1 J a)
		//   b:
		//      <statement>
		//                   J d
		//   c:
		string lb1 = newLabel();
		string lb2 = newLabel();
		string lb3 = newLabel();
		string lb4 = newLabel();
		Lexer::match(For);
		if (Lexer::match(Lpar)) error_num++;
		if (Lexer::token == Id) {
			string name = Lexer::sym;
			if (Lexer::match(Id)) error_num++;
			if (Lexer::match(Assign)) error_num++;
			int type;
			string ename;
			if (expression(type, ename))
				error(20);
			inscode("ASSIGN", ename, name, "-");
		}
		if (Lexer::match(Semicolon)) error_num++;
		inscode(lb1 + ":", "-", "-", "-");
		condition(lb2, lb3);
		if (Lexer::match(Semicolon)) error_num++;
		if (Lexer::token == Id) {
			inscode(lb4 + ":", "-", "-", "-");
			string name = Lexer::sym;
			int len = 0;
			Lexer::match(Id);
			if (Lexer::match(Assign)) {
				error(3);
				Lexer::match(Semicolon);
				return -1;
			}
			string name2 = Lexer::sym;
			if (Lexer::match(Id)) error_num++;
			if (Lexer::token == Add || Lexer::token == Sub) {
				string ins = Lexer::compIns;
				Lexer::match(Lexer::token);
				len = Lexer::ivalue;
				Lexer::match(Num);
				inscode(ins, name2, to_string(len), name);
			}
			else error(15);
		}
		if (Lexer::match(Rpar)) error_num++;
		inscode("J", lb1, "-", "-");
		inscode(lb2 + ":", "-", "-", "-");
		statement();
		inscode("J", lb4, "-", "-");
		inscode(lb3 + ":", "-", "-", "-");

		if (PARSER_CHECK)
			cout << "For \n";
	}
	else if (Lexer::token == Lcur) {
		// { <statement> }
		Lexer::match(Lcur);
		while (Lexer::token != Rcur) {
			if (Lexer::ch == EOF) return-1;
			statement();
		}
		if (Lexer::match(Rcur)) error_num++;
		if (PARSER_CHECK)
			cout << "Statements \n";
	}
	else if (Lexer::token == Return) {
		// return (expression);
		Lexer::match(Return);
		int type;
		string ename = "-";
		if (curTable->rtype == VOID) {
			if (Lexer::match(Semicolon)) {
				error(16);
				Lexer::match(Semicolon);
			}
		}
		else {
			if (Lexer::match(Lpar))
				error(16);
			else {
				if (expression(type, ename))
					error(17);
				else if (curTable->rtype != type)
					error(13);
				if (Lexer::match(Rpar)) error_num++;
			}
			if (Lexer::match(Semicolon)) {
				skipTo(1, Semicolon);
				Lexer::match(Semicolon);
			}
		}
		inscode("RETURN", ename, "-", "-");

		if (PARSER_CHECK)
			cout << "Return \n";
	}
	else if (Lexer::token == Semicolon) {
		Lexer::match(Semicolon);
	}
	else if (Lexer::token == Id) {
		// a = b; a[1]=b; or function_call();
		string name = Lexer::sym;
		Lexer::match(Id);
		if (curTable->isVar(name) || curTable->isConst(name)) { // assign
			if (curTable->isConst(name))
				error(6);
			else {
				if (Lexer::match(Assign)) error_num++;
				int type;
				string ename;
				if (expression(type, ename))
					error(20);
				inscode("ASSIGN", ename, name, "-");
			}
		}
		else if (curTable->isArray(name)) { // array
			Lexer::match(Lbra);
			int type;
			string offset;
			if (expression(type, offset))
				error(20);
			else {
				if (Lexer::match(Rbra)) error_num++;
				if (Lexer::match(Assign)) error_num++;
				string ename;
				if (expression(type, ename))
					error(20);
				else {
					if (curTable->getType(name) != type)
						error(14);
					inscode("STOA", name, offset, ename);
				}
			}
		}
		else if (curTable->isFunc(name)) { // call void function
			if (Lexer::match(Lpar)) error_num++;
			Titem cfunc = curTable->getTitem(name);
			int realargs = 0;
			argsPass(name, realargs);
			if (realargs != cfunc.value) { // wrong args number
				error(7);
				Lexer::match(Semicolon);
				return -1;
			}
			if (Lexer::match(Rpar)) error_num++;
			inscode("CALL", name, "-", "-");
		}

		if (Lexer::match(Semicolon)) {
			skipTo(1, Semicolon);
			Lexer::match(Semicolon);
		}

		if (PARSER_CHECK)
			cout << "Assign \n";
	}
	else if (Lexer::token == Scanf) {
		// scanf(id{,id});
		Lexer::match(Scanf);
		Lexer::match(Lpar);
		string name = Lexer::sym;
		// check table
		if (curTable->getType(name) == INT)
			inscode("SCANI", name, "-", "-");
		else if (curTable->getType(name) == CHAR)
			inscode("SCANC", name, "-", "-");
		else {
			error(10);
			Lexer::match(Semicolon);
			return -1;
		}
		Lexer::match(Id);
		while (Lexer::token == Comma) {
			if (Lexer::ch == EOF) return-1;
			Lexer::match(Comma);
			name = Lexer::sym;
			// check table
			if (curTable->getType(name) == INT)
				inscode("SCANI", name, "-", "-");
			else if (curTable->getType(name) == CHAR)
				inscode("SCANC", name, "-", "-");
			else { error(10); break; }
			Lexer::match(Id);
		}
		Lexer::match(Rpar);
		Lexer::match(Semicolon);

		if (PARSER_CHECK)
			cout << "SCANF \n";
	}
	else if (Lexer::token == Printf) {
		// printf("abc");
		// printf(expression);
		// printf("abc",expression);
		Lexer::match(Printf);
		Lexer::match(Lpar);
		string name = Lexer::sym;
		if (Lexer::token == String) {
			int have = 0;
			int index = -1;
			for (map<string, string>::iterator it = strData.begin();
				it != strData.end();
				it++) {
				++index;
				if (it->second == Lexer::sym) {
					have = 1;
					break;
				}
			}
			string stridx;
			if (have)
				stridx = "$str" + to_string(index);
			else {
				stridx = "$str" + to_string(strData.size());
				strData[stridx] = Lexer::sym;
			}
			inscode("PRTS", stridx, "-", "-");
			Lexer::match(String);
			if (Lexer::token == Comma) {
				Lexer::match(Comma);
				int type;
				string ename;
				if (expression(type, ename)) {
					error(21);
					Lexer::match(Semicolon);
					return -1;
				}
				else if (type == CHAR)
					inscode("PRTC", ename, "-", "-");
				else if (type == INT)
					inscode("PRTI", ename, "-", "-");
				else {
					error(5);
					Lexer::match(Semicolon);
					return -1;
				}
			}
		}
		else {
			int type;
			string ename;
			if (expression(type, ename)) {
				error(21);
				Lexer::match(Semicolon);
				return -1;
			}
			else if (type == CHAR)
				inscode("PRTC", ename, "-", "-");
			else if (type == INT)
				inscode("PRTI", ename, "-", "-");
			else {
				error(5);
				Lexer::match(Semicolon);
				return -1;
			}
		}
		Lexer::match(Rpar);
		if (Lexer::match(Semicolon)) {
			skipTo(1, Semicolon);
			Lexer::match(Semicolon);
		}
		if (PARSER_CHECK)
			cout << "Printf \n";
	}
	else {
		error(3);
		while (Lexer::ch != EOF && (Lexer::token != Rcur) | (Lexer::token != Semicolon)) Lexer::next();
	}
	return 0;
}

// <条件> ::=  <表达式><关系运算符><表达式>|<表达式> //表达式为0条件为假，否则为真
int Parser::condition(string suc, string fail)
{
	int type;
	string name1;
	string name2;
	if (expression(type, name1)) {
		error(22);
		skipTo(2, Rpar, Semicolon);
		return -1;
	}

	if (Lexer::isComp()) {
		string ins = Lexer::compIns;
		string oins = Lexer::opCompIns;
		Lexer::match(Lexer::token);
		if (expression(type, name2)) {
			error(22);
			skipTo(2, Rpar, Semicolon);
			return -1;
		}
		if (suc != "")
			inscode(ins, name1, name2, suc);
		if (fail != "")
			inscode(oins, name1, name2, fail);
	}
	else {
		if (suc != "")
			inscode("BNE", name1, "$0", suc);
		if (fail != "")
			inscode("BEQ", name1, "$0", fail);
	}
	if (PARSER_CHECK)
		cout << "condition \n";
	return 0;
}
//＜表达式＞ :: = ［＋｜－］＜项＞{ ＜加法运算符＞＜项＞ }
int Parser::expression(int&type, string &res)
{
	string op;
	string temp;
	string t1;
	res = "";
	if (Lexer::token == Add || Lexer::token == Sub) {
		op = Lexer::compIns;
		Lexer::match(Lexer::token);
		res = newName();
	}

	if (term(type, temp)) return -1;

	if (res != "") {
		inscode(op, "$0", temp, res);
		t1 = res;
	}
	else t1 = temp;
	while (Lexer::token == Add || Lexer::token == Sub) {
		if (Lexer::ch == EOF) return-1;
		op = Lexer::compIns;
		Lexer::match(Lexer::token);
		if (term(type, temp)) return -1;

		res = newName();
		inscode(op, t1, temp, res);
		t1 = res;
	}

	if (res == "")//not calc in terms.
		res = temp;
	else
		type = INT;
	if (PARSER_CHECK)
		cout << "expression \n";
	return 0;
}
//＜项＞ :: = ＜因子＞{ ＜乘法运算符＞＜因子＞ }
int Parser::term(int&type, string&res)
{
	string op;
	string temp;
	string t1;
	res = "";
	if (factor(type, temp)) return -1;
	t1 = temp;
	while (Lexer::token == Mul || Lexer::token == Div) {
		if (Lexer::ch == EOF) return-1;
		op = Lexer::compIns;
		Lexer::match(Lexer::token);
		if (factor(type, temp)) return -1;

		res = newName();
		inscode(op, t1, temp, res);
		t1 = res;
	}
	// genarate code
	if (res == "")
		res = temp;
	else
		type = INT;

	if (PARSER_CHECK)
		cout << "term \n";
	return 0;
}
// ＜因子＞ ::= ＜标识符＞|＜标识符＞'['＜表达式＞']'|＜整数＞ | ＜字符＞|＜有返回值函数调用语句＞ | '('＜表达式＞')'
int Parser::factor(int&type, string&res)
{
	int oe = error_num;
	if (Lexer::token == Id) { // variable const array function
		string name = Lexer::sym;
		Lexer::match(Id);
		if (curTable->isArray(name)) {// array
			if (Lexer::match(Lbra)) error_num++;
			string offset;
			if (expression(type, offset)) {
				error(23);
				skipTo(1, Rbra);
			}
			type = curTable->getType(name);
			res = newName();
			inscode("GETA", name, offset, res);
			if (Lexer::match(Rbra)) error_num++;
		}
		else if (curTable->isFunc(name)) { // function
			callFunc(name, type, res);
		}
		else if (curTable->isVar(name) || curTable->isConst(name)) { // variable or const
			res = name;
			type = curTable->getType(name);
		}
		else error(12);
	}
	else if (Lexer::token == Num) {
		type = INT;
		res = to_string(Lexer::ivalue);
		Lexer::match(Num);
	}
	else if (Lexer::token == Add || Lexer::token == Sub) {
		int x = Lexer::token == Add ? 1 : -1;
		Lexer::match(Lexer::token);
		if (Lexer::token != Num) error(11);
		type = INT;
		res = to_string(Lexer::ivalue);
		if (x < 0) res = '-' + res;
		Lexer::match(Num);
	}
	else if (Lexer::token == Asc) {
		type = CHAR;
		res = to_string(Lexer::ivalue);
		Lexer::match(Asc);
	}
	else if (Lexer::token == Lpar) {
		if (Lexer::match(Lpar)) error_num++;
		expression(type, res);
		if (Lexer::match(Rpar)) error_num++;
	}
	else error(3);

	if (PARSER_CHECK)
		cout << "factor \n";
	if (oe != error_num) return -1;
	return 0;
}

int Parser::callFunc(string name, int & type, string & res)// for return value func
{
	Titem cfunc = curTable->getTitem(name);
	type = cfunc.type;
	res = '$' + name;//for func return

	if (Lexer::match(Lpar)) error_num++;
	int realargs = 0;
	argsPass(name, realargs);
	if (realargs != cfunc.value) // args number error
		error(7);
	if (Lexer::match(Rpar)) error_num++;

	inscode("CALL", name, "-", "-");

	string tpname = newName();
	tpname[1] = '$';
	inscode("ASSIGN", res, tpname, "-");
	res = tpname;

	return 0;
}

int Parser::argsPass(string name, int & argNum)
{
	if (Lexer::token == Rpar) {
		argNum = 0;
		return 0;
	}
	argNum = 1;
	int type;
	string ename;
	expression(type, ename);
	Titem cfunc = curTable->getTitem(name);
	inscode("PUSH", ename, name, "-");
	while (Lexer::token == Comma) {
		if (Lexer::ch == EOF) return-1;
		Lexer::match(Comma);
		expression(type, ename);
		inscode("PUSH", ename, name, "-");
		argNum++;
	}
	return 0;
}

int Parser::ttp2tp(int & type)
{
	if (type == Int)type = INT;
	else if (type == Char)type = CHAR;
	else if (type == Void)type = VOID;
	else return -1;
	return 0;
}

int Parser::error(int error_no)
{
	cout << Lexer::lineno << " : ";
	switch (error_no)
	{
	case 0:cout << "未处理处理！\n"; break;
	case 1:cout << "程序结尾错误！\n"; break;
	case 2:cout << "变量定义错误\n"; break;
	case 3:cout << "语句格式错误\n"; break;
	case 4:cout << "变量名重复\n"; break;
	case 5:cout << "打印错误类型\n"; break;
	case 6:cout << "赋值左值不为变量\n"; break;
	case 7:cout << "参数个数错误\n"; break;
	case 8:cout << "未定义函数\n"; break;
	case 9:cout << "除数为0\n"; break;
	case 10:cout << "读取错误\n"; break;
	case 11:cout << "整数格式错误\n"; break;
	case 12:cout << "未定义标识符\n"; break;
	case 13:cout << "警告：返回类型错误\n"; return 0; break;
	case 14:cout << "警告：赋值类型错误\n"; return 0; break;
	case 15:cout << "步长计算方式错误\n"; break;
	case 16:cout << "返回语句格式错误\n"; break;
	case 17:cout << "返回语句中表达式错误\n"; break;
	case 18:cout << "常量定义错误\n"; break;
	case 19:cout << "函数参数定义错误\n";  break;
	case 20:cout << "表达式错误\n"; break;
	case 21:cout << "输出语句表达式错误\n"; break;
	case 22:cout << "条件语句表达式错误\n"; break;
	case 23:cout << "数组偏移量错误\n"; break;
	default:
		break;
	}
	skipTo(1, Semicolon);
	return 0;
}

int Parser::inscode(string s1, string s2, string s3, string s4) {
	midcodes.push_back(Midcode(s1, s2, s3, s4));
	return 0;
}

int Parser::skipTo(int count, ...)
{
	error_num++;
	va_list args;
	while (1) {
		va_start(args, count);
		for (int i = 0; i < count; i++)
			if (Lexer::token == va_arg(args, int))return 0;
		Lexer::next();
		if (Lexer::ch == EOF) return-1;
	}
	return 0;
}