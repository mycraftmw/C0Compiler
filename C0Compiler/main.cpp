#include <iostream>
#include <fstream>
#include "defines.h"
#include "Lexer.h"
#include "Parser.h"
#include "Titem.h"
#include "Table.h"
#include "Midcode.h"
#include "Optimizer.h"
#include "Generator.h"
#include "GlobalVar.h"

using namespace std;

int main(int argc, char *argv[]) {
	string basename;
	bool isOptimize = false;
	argc--;
	if (argc < 1) {
		printf("please input file\n");
		return 0;
	}
	else if (argc == 1) {
		basename.assign(argv[1]);
	}
	else if (argc == 2) {
		if (argv[1][0] == '-' && argv[1][1] == 'o')
			isOptimize = true;
		basename.assign(argv[2]);
	}

	// init
	Lexer::init(basename);
	Parser::init("midcode.txt");
	Generator::init("mipscode.asm");
	curTable = new Table("$");
	allTable.push_back(curTable);

	// begin parser
	Parser::program();

	if (Parser::error_num == 0)cout << "error_num: " << Parser::error_num << endl;
	if (Parser::error_num == 0) {
		Parser::outputMidcode();
		if (isOptimize) {
			Optimizer::init("optmidcode.txt");
			Optimizer::optimize(Parser::midcodes);
			Optimizer::outputOptcode();
			Generator::generateMips(Optimizer::optcodes);
		}
		else {
			Generator::generateMips(Parser::midcodes);
		}
	}
	return 0;
}