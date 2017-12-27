#ifndef GENERATOR_H
#define GENERATOR_H
#include <vector>
#include <fstream>
#include "Midcode.h"
using namespace std;

class Generator
{
public:
	static ofstream out;
	static size_t codeLine;//cur line number

	static int init(string filepath);
	static int generateMips(vector<Midcode> midcodes);
	static bool laterUse(string regName, int n);
	static void saveRegs(int st, int ed);
	static void restoreRegs(int st, int ed);
};

#endif // !GENERATOR_H
