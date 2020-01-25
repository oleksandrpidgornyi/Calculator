#include <iostream>
#include <iterator>
#include <stack>
#include <sstream>
#include <cmath>

#include "CCalculator.h"
#include "CLogger.h"

#define IS_OPERATION(x) (x == '+' || x == '-' || x == '*' || x == '/' || x == '^')
#define IS_DIGIT(x) (x >= '0' && x <= '9')
#define IS_NUMBER(x) (IS_DIGIT(x) || x == '.' || x == ',')
#define IS_SPACE(x) (x  == ' ' || x  == '\t')

vector<string> test_expr = {
    "3 + 4 * 2 / (1 - 5) ^ 2 ^ 3",
    "((15 / (7 - (1 + 1))) * 3) - (2 + (1 + 1))",
    "22+33*44",
    " - 22 +   33*44",
    "22+33)*44",
    "22+33 44",
    "22+33 44 55",
    "22+33**44",
    "    22+33*44",
    "22+33*44    ",
    "22+33    *    44",
    "\t\t\t22+33*44",
    "22+33*\t\t\t44",
    "22+33*44\t\t\t",
};

unsigned int CCalculator::GetToken(const string& expr, unsigned int start, CToken& token)
{
    bool check_sign = false;
    if (start == 0) {
        check_sign = true;
    }
    while (IS_SPACE(expr[start])) {
        start++;
    }
    if (start == expr.size()) {
        return 0;
    }
    unsigned int i = start;
    if (IS_DIGIT(expr[i])) {
        //number
        while (IS_NUMBER(expr[i])) {
            i++;
        }
        token.sval = expr.substr(start, i - start);
        token.tok = TOKENS::Number;
        istringstream(token.sval) >> token.dval;
    }
    else if (expr[i] == '(' || expr[i] == ')') {
        //expression
        i++;
        token.sval = expr.substr(start, i - start);
        token.dval = token.sval[0];
        token.tok = TOKENS::Expr;
    }
    else if (IS_OPERATION(expr[i])) {
        //operation
        token.sval = expr.substr(start, 1);
        token.tok = TOKENS::Operator;
        i++;
        switch (token.sval[0]) {
        case '+':
            token.dval = 1;//priority
            break;
        case '-':
            token.dval = 1;
            if (check_sign) {
                i = GetToken(expr, i, token);
                token.dval = -token.dval;
            }
            break;
        case '*':
            token.dval = 2;
            break;
        case '/':
            token.dval = 2;
            break;
        case '^':
            token.dval = 3;
            break;
        default:
            LOGE("Wrong operation = %s\n", token.sval.c_str());
            return -1;
        }
    }
    if (token.tok == TOKENS::Number) {
        LOGD("len=%d token = %f\n", i, token.dval);
    }
    else {
        LOGD("len=%d token = %s\n", i, token.sval.c_str());
    }
    return i;
}

//Operator	Precedence	Associativity
//   ^         4        Right
//   ×         3        Left
//   ÷         3        Left
//   +         2        Left
//   −         2        Left
//   (         1        ???
//   )         1        ???

void CCalculator::InfixToPostfix()
{
    //written due to wikipedia article
    //https://en.wikipedia.org/wiki/Shunting-yard_algorithm
    //a mistake with ^ operator associativity was fixed
    //
    //This implementation does not implement composite functions,
    //unctions with variable number of arguments, and unary operators.
    stack<CToken> oper;
    for (auto x : infix) {
        //while there are tokens to be read do:
        //read a token.
        if (x.tok == TOKENS::Number) {
            //if the token is a number, then :
            //push it to the output queue.
            postfix.push(x);
            LOGD("push: %f\n", x.dval);
        }
        else if (x.tok == TOKENS::Function) {
            //if the token is a function then :
            //push it onto the operator stack
        }
        else if (x.tok == TOKENS::Operator) {
            //if the token is an operator, then :
            //while (
            while (oper.size()
                //    (there is a function at the top of the operator stack)
                && (oper.top().tok == TOKENS::Function
                    // or (there is an operator at the top of the operator stack with greater precedence)
                    || (oper.top().tok == TOKENS::Operator
                        && oper.top().dval >= x.dval)
                    //                // or (the operator at the top of the operator stack has equal precedence and is left associative)) - ???
                    //                || (   oper.top().tok  == TOKENS::Operator
                    //                    && oper.top().dval == x.dval
                    //                    && oper.top().sval[0] != '^')//'^' is right associative
                                    // and (the operator at the top of the operator stack is not a left parenthesis) :
                    ))
            {
                //pop operators from the operator stack onto the output queue.
                postfix.push(oper.top());
                LOGD("push: %s\n", oper.top().sval.c_str());
                oper.pop();
            }
            //push it onto the operator stack.
            oper.push(x);
        }
        else if (x.tok == TOKENS::Expr && x.sval[0] == '(') {
            //if the token is a left paren(i.e. "("), then :
            //push it onto the operator stack.
            oper.push(x);
        }
        else if (x.tok == TOKENS::Expr && x.sval[0] == ')') {
            //if the token is a right paren(i.e. ")"), then :
            //while the operator at the top of the operator stack is not a left paren :
            while (oper.top().sval[0] != '(') {
                //pop the operator from the operator stack onto the output queue.
                postfix.push(oper.top());
                LOGD("push: %s\n", oper.top().sval.c_str());
                oper.pop();
            }
            /* if the stack runs out without finding a left paren, then there are mismatched parentheses. */
            //if there is a left paren at the top of the operator stack, then :
            if (oper.top().sval[0] == '(') {
                //pop the operator from the operator stackand discard it
                oper.pop();
            }
        }
    }
    /* After while loop, if operator stack not null, pop everything to output queue */
    //if there are no more tokens to read then :
    //while there are still operator tokens on the stack :
    while (oper.size()) {
        /* if the operator token on the top of the stack is a paren, then there are mismatched parentheses. */
        //pop the operator from the operator stack onto the output queue.
        postfix.push(oper.top());
        LOGD("push: %s\n", oper.top().sval.c_str());
        oper.pop();
    }
    //exit.
}

double CCalculator::CalculateOperation(CToken& op, CToken& a, CToken& b)
{
    double result = 0;
    switch (op.sval[0]) {
    case '+':
        result = a.dval + b.dval;
        break;
    case '-':
        result = a.dval - b.dval;
        break;
    case '*':
        result = a.dval * b.dval;
        break;
    case '/':
        result = a.dval / b.dval;
        break;
    case '^':
        result = pow(a.dval, b.dval);
        break;
    }
    LOGD("%f%c%f=%f\n", a.dval, op.sval[0], b.dval, result);
    return result;
}

double CCalculator::PostfixEvaluate()
{
    //written due to wikipedia article
    //https://en.wikipedia.org/wiki/Reverse_Polish_notation
    stack<CToken> oper;
    //for each token in the postfix expression :
    while (postfix.size()) {
        CToken& t = postfix.front();
        //if token is an operator :
        if (t.tok == TOKENS::Operator) {
            //operand_2 ← pop from the stack
            CToken& b = oper.top();
            oper.pop();
            //operand_1 ← pop from the stack
            CToken& a = oper.top();
            oper.pop();
            //result ← evaluate token with operand_1and operand_2
            CToken c;
            c.tok = TOKENS::Number;
            c.dval = CalculateOperation(t, a, b);
            //push result back onto the stack
            oper.push(c);
        }
        //else if token is an operand :
        else if (t.tok == TOKENS::Number) {
            //push token onto the stack
            oper.push(t);
        }
        postfix.pop();
    }
    //result ← pop from the stack
    double result = oper.top().dval;
    oper.pop();
    return result;
}

CCalculator::CCalculator()
{
}

CCalculator::~CCalculator()
{
}

string CCalculator::GetExpression()
{
	string str;
    string input;
    string expr;
    cout << COLOR_L_YELLOW_TEXT << "Input expression to calculate:" << COLOR_END << endl;
    while (1) {
        input.clear();
        getline(cin, input);
        LOGD("input=%s\n", input.c_str());
        if (input.empty() && expr.empty()) {
            LOGD("exiting\n");
            break;
        }
        else
        {
            expr = expr + input;
            if (input.back() == '=') {
                expr.pop_back();
                LOGD("processing expr=%s\n", expr.c_str());
                break;
            }
        }
    }
    return expr;
}

int CCalculator::Run(bool test)
{
    cout << COLOR_YELLOW_TEXT "This is a simple line expression calculator." COLOR_END << endl;
    if (test) {
        cout << COLOR_L_BLUE_TEXT "Test mode." COLOR_END << endl << endl;
    }
    else {
        cout << COLOR_YELLOW_TEXT "Type expression with '=' at the end to calculate," COLOR_END << endl;
        cout << COLOR_YELLOW_TEXT "or press 'Enter' to exit." COLOR_END << endl << endl;
    }
    unsigned int i = 0;
    while (1) {
        string expr;
        if (test) {
            if (i == test_expr.size()) {
                return 0;
            }
            expr = test_expr[i++];
        }
        else {
            expr = GetExpression();
        }
        cout << COLOR_YELLOW_TEXT << "Expression to calculate: " << expr << COLOR_END << endl << endl;
        if (expr == "") {
            cout << COLOR_RED_TEXT "Press 'Enter' to exit." COLOR_END << endl << endl;
            expr.clear();
            getline(cin, expr);
            LOGD("expr = %s\n", expr.c_str());
            if (expr == "") {
                return 0;
            }
        } else {
            infix.clear();
            int res = ParseStringToInfix(expr, 0, expr.length());
            if (res == 0) {
                InfixToPostfix();
                double result = PostfixEvaluate();
                cout << COLOR_GREEN_TEXT "result = " << result << COLOR_END << endl << endl;
            }
        }
    }
    return 0;
}

int CCalculator::ParseStringToInfix(string& expr, unsigned int start, unsigned int length)
{
    CToken token;
    LOGD("before: expr = %s, length=%d\n", expr.c_str(), length);
    while (start < length) {
        start = GetToken(expr, start, token);
        if (start < 0) {
            cout << COLOR_RED_TEXT "Wrong operation sign." COLOR_END << endl << endl;
            return -1;
        }
        else if (start > 0) {
            infix.push_back(token);
        }
        else {
            //trailing spaces
            LOGD("trailing spaces\n");
            break;
        }
    }
    //check parenthesis number
    int cnt = 0;
    for (auto & t : infix) {
        if (t.sval[0] == '(') cnt++;
        else if (t.sval[0] == ')') cnt--;
    }
    if (cnt) {
        cout << COLOR_RED_TEXT "Wrong number of parenthesis." COLOR_END << endl << endl;
        return -1;
    }
    //check expression order
    int op_cnt = 0;
    int num_cnt = 0;
    for (auto& t : infix) {
        if (t.tok == TOKENS::Number) num_cnt++;
        else if (t.tok == TOKENS::Operator) op_cnt++;
    }
    if (num_cnt - op_cnt != 1) {
        cout << COLOR_RED_TEXT "Wrong expression." COLOR_END << endl << endl;
        return -1;
    }
    return 0;
}

