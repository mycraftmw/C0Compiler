#ifndef DEFINES_H
#define DEFINES_H
#include <string>
#include <map>

using namespace std;

// token type
enum {
	Char,
	Const,
	Do,
	Else,
	For,
	If,
	Int,
	Main,
	Printf,
	Return,
	Scanf,
	Void,
	While,
	Num,
	Asc,
	String,
	Id,
	Assign,
	Eq,
	Ne,
	Lt,
	Gt,
	Le,
	Ge,
	Add,
	Sub,
	Mul,
	Div,
	Lcur,
	Rcur,
	Lbra,
	Rbra,
	Lpar,
	Rpar,
	Comma,
	Semicolon,
	Notype
};

// Titem kind

enum {
	CONST,
	VAR,
	ARRAY,
	FUNC,
	PARAM
};

// var type
enum {
	INT,
	CHAR,
	VOID
};

static std::string Titem_words[] = {
	"CONST",
	"VAR",
	"ARRAY",
	"FUNC",
	"PARAM"
};

static std::string type_words[] = {
	"INT",
	"CHAR",
	"VOID"
};

static std::string symbol_words[] = {
	"Char",
	"Const",
	"Do",
	"Else",
	"For",
	"If",
	"Int",
	"Main",
	"Printf",
	"Return",
	"Scanf",
	"Void",
	"While",
	"Num",
	"Asc",
	"String",
	"Id",
	"Assign",
	"Eq",
	"Ne",
	"Lt",
	"Gt",
	"Le",
	"Ge",
	"Add",
	"Sub",
	"Mul",
	"Div",
	"Lcur",
	"Rcur",
	"Lbra",
	"Rbra",
	"Lpar",
	"Rpar",
	"Comma",
	"Semicolon",
	"Notype"
};

#endif // !DEFINES_H
