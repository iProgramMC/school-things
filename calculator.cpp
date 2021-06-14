#include<iostream>
#include<string>
#include<sstream>
#include<stack>
#include<vector>
using namespace std;

enum {
    TYPE_NONE,TYPE_NUMBER,TYPE_OPERATOR,

    TYPE_EOF=255
};
enum {
    OP_NONE,OP_ADD,OP_SUB,OP_MUL,OP_DIV
};
struct Token {
    int type,value;
};
int g_error, g_head, g_errorLength;
enum {
    ERROR_UNKNOWN,
    ERROR_EXPECTED_NUMBER,
    ERROR_EXPECTED_OPERATOR,
    ERROR_UNKNOWN_CHARACTER,
    ERROR_DIVIDE_BY_ZERO,
    ERROR_CALCULATION_FAILURE
};
std::string g_errorNames[] = {
    "Unknown error",
    "Expected number",
    "Expected operator",
    "Unknown character",
    "Cannot divide by zero",
    "Calculation failure",
};
#ifdef DEBUG
#define OnError(Head, ErrorCode, ...) do { cout<<#ErrorCode; g_error = ErrorCode; g_head = Head; g_errorLength = strlen (#ErrorCode); return __VA_ARGS__; } while (0)
#else
#define OnError(Head, ErrorCode, ...) do { cout<<g_errorNames[ErrorCode]; g_error = ErrorCode; g_errorLength = g_errorNames[ErrorCode].size(); g_head = Head; return __VA_ARGS__; } while (0)
#endif
//helper to turn anything into a string, like ints/floats
template< class C>
std::string toString(C value)
{
	std::ostringstream o;
	o << value;
	return o.str();
}

std::stack<Token> g_output, g_opStack;

int g_precedence[] = { 0, 1, 1, 2, 2 }; // mul&div have higher precedence than add&sub
bool IsPrecedenceLower(int opType1, int opType2) {
    return(g_precedence[opType1] < g_precedence[opType2]);
}

void PushTokenToOutput(Token t) {
    g_output.push(t);
}
void PushTokenToOpStack(Token t) {
    g_opStack.push(t);
}
int GetLatestOperator() {
    if (g_opStack.empty()) return OP_NONE;
    return g_opStack.top().value;
}
void MoveLatestOperatorToOutput() {
    g_output.push(g_opStack.top());
    g_opStack.pop();
}
std::string g_operatorNames[] = {
    "NON","ADD","SUB","MUL","DIV"
};
void PrintToken(Token t) {
    std::string typeToStr, valueToStr;
    switch(t.type) {
        case TYPE_NONE: typeToStr = "???", valueToStr = "???"; break;
        case TYPE_NUMBER: typeToStr = "NUM", valueToStr = toString<int>(t.value); break;
        case TYPE_OPERATOR: typeToStr="OPR", valueToStr = g_operatorNames[t.value]; break;
        case TYPE_EOF: typeToStr = "EOF", valueToStr = "///"; break;

    }
    cout<<"{"<<typeToStr<<": "<<valueToStr<<"}";
}
void PrintDebug() {
    /*
    std::stack<Token> temp;//we want to read stack, but not erase stuff inside
    cout<<"output   stack: ";
    while (!g_output.empty()) {
        temp.push(g_output.top()); g_output.pop();
    }
    while (!temp.empty()) {
        g_output.push(temp.top());
        PrintToken(temp.top());
        temp.pop();
    }
    cout<<'\n';
    cout<<"operator stack: ";
    while (!g_opStack.empty()) {
        temp.push(g_opStack.top()); g_opStack.pop();
    }
    while (!temp.empty()) {
        g_opStack.push(temp.top());
        PrintToken(temp.top());
        temp.pop();
    }*/
    //cout<<'\n';
    //cout<<'\n';
}

std::vector<Token> StackToVector() {
    std::vector<Token> ts;
    std::stack<Token> temp;

    while(!g_output.empty()) {
        temp.push(g_output.top());
        g_output.pop();
    }
    while(!temp.empty()) {
        ts.push_back(temp.top()); temp.pop();
    }
    return ts;
}

void OnFinishThisToken(Token *t) {
    if (t->type == TYPE_NUMBER) {
        PushTokenToOutput(*t);
        PrintDebug();
    }
    if (t->type == TYPE_OPERATOR) {
        if (IsPrecedenceLower(t->value, GetLatestOperator())) {
            // push everything to operator stack
            while (!g_opStack.empty())
                MoveLatestOperatorToOutput();
        }
        PushTokenToOpStack(*t);
        PrintDebug();
    }
    if (t->type == TYPE_EOF) {
        while (!g_opStack.empty())
            MoveLatestOperatorToOutput();

        PrintDebug();
    }
    t->type = TYPE_NONE;
    t->value = 0;
}
void ParseOperation(std::string op) {
    op = op + "  ";
    Token t;
    t.type = TYPE_NONE;
    t.value = 0;
    int last_token_type = TYPE_NONE;
    char c; int head=0;
    int sz = op.size();
    while /* we have tokens to read */
    (head < sz)
    {
        c = op[head];
        head++;
        if (isspace(c)) {
            last_token_type = t.type;
            OnFinishThisToken(&t);
        }
        else if (isdigit(c)) {
            if (t.type == TYPE_NONE && last_token_type == TYPE_NUMBER)
                OnError(head, ERROR_EXPECTED_OPERATOR);
            if (t.type != TYPE_NONE && t.type != TYPE_NUMBER) {
                last_token_type = t.type;
                OnFinishThisToken(&t);
            }
            t.type = TYPE_NUMBER;
            t.value = (t.value * 10) + (c - '0');
        }
        else {
            if (t.type == TYPE_NONE && last_token_type == TYPE_OPERATOR)
                OnError(head, ERROR_EXPECTED_NUMBER);
            if (t.type != TYPE_NONE && t.type != TYPE_OPERATOR) {
                last_token_type = t.type;
                OnFinishThisToken(&t);
            }
            t.type = TYPE_OPERATOR;
            switch (c) {
                case'+':t.value=OP_ADD;break;
                case'-':t.value=OP_SUB;break;
                case'*':t.value=OP_MUL;break;
                case'/':t.value=OP_DIV;break;
                default:OnError(head, ERROR_UNKNOWN_CHARACTER);
            }
            //! Operators are just one character
            last_token_type = t.type;
            OnFinishThisToken(&t);
        }
    }
    if (last_token_type == TYPE_OPERATOR)
        OnError(head-1, ERROR_EXPECTED_NUMBER);
    t.type = TYPE_EOF;
    t.value = 0;
    OnFinishThisToken(&t);
}
std::vector<Token> g_tokens;
void PrintDebugList() {
    for(int i=0; i<g_tokens.size(); i++)
        PrintToken(g_tokens[i]);
}
void ParseRPN() {
    g_tokens = StackToVector();

    int hd = 0;
    for (; hd != g_tokens.size(); hd++) {
        Token* t = &g_tokens[hd];
        if (t->type == TYPE_OPERATOR) {
            //cout<<"Operating with type "<<g_operatorNames[t->value]<<'\n';
            // take the Last 2 values

            if (hd < 2) OnError(-1, ERROR_EXPECTED_NUMBER);

            Token *tb1 = &g_tokens[hd-1];
            Token *tb2 = &g_tokens[hd-2];

            if (tb1->type != TYPE_NUMBER) {
                OnError(-1, ERROR_EXPECTED_NUMBER);
            }
            if (tb2->type != TYPE_NUMBER) {
                OnError(-1, ERROR_EXPECTED_NUMBER);
            }

            // and process the result
            int result = tb2->value;
            switch (t->value) {
                case OP_ADD:result+=tb1->value;break;
                case OP_SUB:result-=tb1->value;break;
                case OP_MUL:result*=tb1->value;break;
                case OP_DIV:if(tb1->value==0)OnError(-1, ERROR_DIVIDE_BY_ZERO);result/=tb1->value;break;
            }

            // tb2 gets the value of the result
            tb2->type = TYPE_NUMBER;
            tb2->value = result;

            // invalidate the pointers, we won't use them anymore
            // and remove the other ones
            g_tokens.erase(g_tokens.begin()+(hd-1));
            g_tokens.erase(g_tokens.begin()+(hd-1));
            //PrintDebugList();
            hd -= 2;
        }
    }
}

void InternalCheckError() {
    if (!g_error) return;
    int p = 18+g_head;
    //if (g_head != -1) return;
    // print head arrow pointer
    for (int i=g_errorLength; i<18; i++)
        cout<<' ';
    //cout<<"#";
    for (int i=0; i<g_head-1; i++)
        cout<<'~';
    cout<<'^';
    cout<<'\n';
}
#define CheckError do { InternalCheckError(); if (g_error) return 0; } while (0)

int main() {
    cout<<"put in operation: ";
    std::string op;
    getline(cin,op);
    ParseOperation(op);
    CheckError;
    // we got the RPN, now parse it
    ParseRPN();
    CheckError;

    if (g_tokens.size() > 1) OnError(-1, ERROR_CALCULATION_FAILURE, 0);
    CheckError;
    if (g_tokens.size() > 0)
        cout<<g_tokens[0].value;
    else
        cout<<"???";
    return 0;
}
