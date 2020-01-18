#pragma once
#include <string>
#include <vector>
#include <queue>

using namespace std;

class CCalculator
{
	enum class TOKENS {
		Unknown,
		Function,
		Operator,
		Number,
		Expr
	};

	struct CToken {
		TOKENS tok;
		double dval;
		string sval;
	};

private:
	//members
	vector<CToken> infix;
	queue<CToken> postfix;
	//methods
	string GetExpression();
	unsigned int GetToken(const string& expr, unsigned int start, CToken& token);
	int ParseStringToInfix(string& expr, unsigned int start, unsigned int end);
	void InfixToPostfix();
	double PostfixEvaluate();
	double CalculateOperation(CToken& op, CToken& a, CToken& b);
public:
	CCalculator();
	~CCalculator();
	int Run(bool test);
};

