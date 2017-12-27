#include <iostream>
#include "defines.h"
#include "GlobalVar.h"
#include "Lexer.h"

ifstream Lexer::in;
char Lexer::ch;
map<string, int> Lexer::reserved_words; //保留字
int Lexer::lineno;

int Lexer::token;
string Lexer::sym;
int Lexer::ivalue;

string Lexer::compIns;
string Lexer::opCompIns;

int Lexer::init(string filepath) {
	reserved_words[string("char")] = Char;
	reserved_words[string("const")] = Const;
	reserved_words[string("do")] = Do;
	reserved_words[string("else")] = Else;
	reserved_words[string("for")] = For;
	reserved_words[string("if")] = If;
	reserved_words[string("int")] = Int;
	reserved_words[string("main")] = Main;
	reserved_words[string("printf")] = Printf;
	reserved_words[string("return")] = Return;
	reserved_words[string("scanf")] = Scanf;
	reserved_words[string("void")] = Void;
	reserved_words[string("while")] = While;
	in.open(filepath);
	lineno = 0;
	ch = '\n';
	return 0;
}
bool Lexer::isChar(int c) {
	return c == '+' || c == '-' || c == '/' || c == '*' || c == '_' || isalnum(c);
}

bool Lexer::isType()
{
	return token == Char || token == Int;
}

bool Lexer::isComp()
{
	return token == Lt || token == Le || token == Eq || token == Ne || token == Ge || token == Gt;
}

int Lexer::readCh() {
	if (ch == '\n') lineno++;
	ch = in.get();
	//if (in.eof()) {
	//	cout << "程序未完成！\n";
	//}
	//cout << ch;
	return ch;
}

int Lexer::next() {
	while (isspace(ch))readCh();
	if (isdigit(ch)) {
		sym = ch;
		while (isdigit(readCh()))
			sym += ch;
		ivalue = stoi(sym);
		if (sym[0] == '0' && ivalue > 0)
			return -1;
		else token = Num;
	}
	else if (isalpha(ch) || ch == '_') {
		sym = ch;
		readCh();
		while (isalnum(ch) || ch == '_') {
			sym += ch;
			readCh();
		}
		if (reserved_words.count(sym))
			token = reserved_words[sym];
		else token = Id;
	}
	else if (ch == '"') {
		sym = "";
		while (readCh() != '"') {
			if (ch < 32 || ch>126) return -1;
			sym += ch;
		}
		readCh();
		token = String;
	}
	else if (ch == '\'') {
		sym = readCh();
		ivalue = ch;
		if (!isChar(ch))return -1;
		if (readCh() != '\'') return -1;
		token = Asc;
		readCh();
	}
	else if (ch == '=') {
		if (readCh() == '=') {
			token = Eq;
			compIns = "BEQ";
			opCompIns = "BNE";
			readCh();
		}
		else token = Assign;
	}
	else if (ch == '<') {
		if (readCh() == '=') {
			token = Le;
			compIns = "BLE";
			opCompIns = "BGT";
			readCh();
		}
		else {
			token = Lt;
			compIns = "BLT";
			opCompIns = "BGE";
		}
	}
	else if (ch == '>') {
		if (readCh() == '=') {
			token = Ge;
			compIns = "BGE";
			opCompIns = "BLT";
			readCh();
		}
		else {
			token = Gt;
			compIns = "BGT";
			opCompIns = "BLE";
		}
	}
	else if (ch == '!') {
		if (readCh() == '=') {
			token = Ne;
			compIns = "BNE";
			opCompIns = "BEQ";
			readCh();
		}
		else return -1;
	}
	else if (ch == '+') {
		token = Add;
		compIns = "ADD";
		readCh();
	}
	else if (ch == '-') {
		token = Sub;
		compIns = "SUB";
		readCh();
	}
	else if (ch == '*') {
		token = Mul;
		compIns = "MUL";
		readCh();
	}
	else if (ch == '/') {
		token = Div;
		compIns = "DIV";
		readCh();
	}
	else if (ch == '(') {
		token = Lpar;
		readCh();
	}
	else if (ch == ')') {
		token = Rpar;
		readCh();
	}
	else if (ch == '{') {
		token = Lcur;
		readCh();
	}
	else if (ch == '}') {
		token = Rcur;
		readCh();
	}
	else if (ch == '[') {
		token = Lbra;
		readCh();
	}
	else if (ch == ']') {
		token = Rbra;
		readCh();
	}
	else if (ch == ',') {
		token = Comma;
		readCh();
	}
	else if (ch == ';') {
		token = Semicolon;
		readCh();
	}
	else
		return -1;
	return 0;
}

int Lexer::match(int tk) {
	if (tk != token) {
		cout << lineno << ": " << " except " << symbol_words[tk] << " but caught " << symbol_words[token] << endl;
		return -1;
	}
	else if (next()) {
		if (ch != -1)
			cout << lineno << ": " << " unexcept symbol : " << (int)ch << " (" << ch << ")" << endl;
		return -1;
	}
	return 0;
}

int Lexer::skipMatch(int tk) {
	if (token != tk)
		cout << lineno << ": " << "error: except " << symbol_words[tk] << " but caught " << symbol_words[token] << endl;
	while (token != tk)
		if (next() && ch == -1)
			return -1;
	return 0;
}