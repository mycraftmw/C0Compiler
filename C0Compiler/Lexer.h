#ifndef LEXER_H
#define LEXER_H
#include <fstream>
#include <map>
using namespace std;
class Lexer
{
public:
	static ifstream in;
	static char ch;
	static map<string, int> reserved_words; //±£Áô×Ö
	static int lineno;

	static int token;
	static string sym;
	static int ivalue;

	static string compIns;
	static string opCompIns;

	static int init(string filepath);
	static bool isChar(int c);
	static bool isType();
	static bool isComp();
	static int readCh();
	static int next();
	static int match(int tk);
	static int skipMatch(int tk);
};

#endif // !LEXER_H
