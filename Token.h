#pragma once

#include <string>
#include <vector>
#include "TokenType.h"

using namespace std;

class Token
{	
public:
	Token();
	Token(TokenType type, string value);
	TokenType type;
	string value;
};

class Keyword
{
public:
	static Token CheckAndCreate(string code, int position);
	static vector<string> posValues;
	static string endSymbols;
	static vector<string> posVars;
};

class Identifier
{
public:
	static Token CheckAndCreate(string code, int position);
	static string endSymbols;
};

class Operator
{
public:
	static Token CheckAndCreate(string code, int position);
	static vector<string> posValues; // Longest operators must be first
};

class Separator
{
public:
	static Token CheckAndCreate(string code, int position);
	static vector<string> posValues; // Longest operators must be first
};

class Literal
{
public:
	static Token CheckAndCreate(string code, int position);
	static vector<string> posValues;
};

class Comment
{
public:
	static Token CheckAndCreate(string code, int position);
	static string commentSymbol;
};

