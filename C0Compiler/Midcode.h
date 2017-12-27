#ifndef MIDCODE_H
#define MIDCODE_H
#include <string>

using namespace std;

class Midcode
{
public:
	string op;
	string src1;
	string src2;
	string dst;

	Midcode(string op, string src1, string src2, string dst);
	bool canReplace(string op);
	string toString();
};

#endif // !MIDCODE_H
