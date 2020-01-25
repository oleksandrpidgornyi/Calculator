// Calc.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <string.h>
#include "CCalculator.h"
#include "CLogger.h"

#ifdef _WIN32
#include <windows.h>

class CColorConsole {
private:
	void* hStdout;
	unsigned long consoleMode;
public:
	CColorConsole() {
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleMode(hStdout, &consoleMode);
		SetConsoleMode(hStdout, consoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	}
	~CColorConsole() {
		SetConsoleMode(hStdout, consoleMode);
	}
};

CColorConsole colCons;
#endif //_WIN32

int main(int argc, char* argv[], char* envp[])
{
	LOG_INIT_COLORCONSOLE;

	bool test = false;
	if (argc == 2) {
		string arg(argv[1]);
		if (arg == "-t") {
			test = true;
		}
	}

	CCalculator* c = new CCalculator;
	int res = c->Run(test);
	delete c;
	return res;
}
