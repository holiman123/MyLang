#include "Token.h"
#include "TokenType.h"
#include <vector>

using namespace std;

vector<string> Keyword::posValues;
vector<string> Keyword::posVars;
string Identifier::endSymbols;
string Keyword::endSymbols;
vector<string> Operator::posValues;
vector<string> Separator::posValues;
string Comment::commentSymbol;
vector<string> Literal::posValues;

Token::Token()
{
	type = TokenType::Identifier;
}

Token::Token(TokenType type, string value)
{
	this->type = type;
	this->value = value;
}

Token Keyword::CheckAndCreate(string code, int position)
{
	Token res;
	for (string pattern : posValues)
	{				
		if (code.substr(position, pattern.length()) == pattern && endSymbols.find(code[position + pattern.length()]) != string::npos)
		{
			res.type = TokenType::Keyword;
			res.value = pattern;
			break;
		}
	}

	return res;
}

Token Identifier::CheckAndCreate(string code, int position)
{		
	int localPos = 0;
	string resValue = "";
	while (endSymbols.find(code[position + localPos]) == string::npos)
	{
		char c = code[position + localPos];
		resValue += c;
		localPos++;
	}

	Token res(TokenType::Identifier, resValue);
	return res;
}

Token Operator::CheckAndCreate(string code, int position)
{	
	Token res;
	for (string op : posValues)
	{
		if (code.substr(position, op.length()) == op)
		{
			res.type = TokenType::Operator;
			res.value = op;
			break;
		}
	}

	return res;
}

Token Separator::CheckAndCreate(string code, int position)
{
	Token res;
	for (string pattern : posValues)
	{
		if (code.substr(position, pattern.length()) == pattern)
		{
			res.type = TokenType::Separator;
			res.value = pattern;
			break;
		}
	}

	return res;
}

Token Literal::CheckAndCreate(string code, int position)
{
	string value = "";	

	if (isdigit(code[position]) || code[position] == '-')
	{
		value += code[position];
		for (size_t i = position + 1; isdigit(code[i]); i++)
		{
			value += code[i];
		}
	}
	
	if (code[position] == '"')
	{
		value += "\"";
		for (size_t i = position + 1; code[i] != '"'; i++)
		{
			value += code[i];
		}
		value += "\"";
	}

	for (string pattern : posValues)
	{
		if (code.substr(position, pattern.length()) == pattern)
		{			
			value = pattern;
			break;
		}
	}

	return Token(TokenType::Literal, value);
}

Token Comment::CheckAndCreate(string code, int position)
{
	Token res;

	if (code[position] == '#' && code[position + 1] == '#')
	{
		string value = "";
		for (size_t i = position; i < code.length(); i++)
		{
			if (code[i] == '\n')
				break;
			value += code[i];
		}

		res = Token(TokenType::Comment, value);
	}

	if (code[position] == '#' && code[position + 1] == '-')
	{
		string value = "";
		for (size_t i = position; i < code.length(); i++)
		{
			value += code[i];
			if (code[i - 1] == '-' && code[i] == '#')
				break;
		}

		res = Token(TokenType::Comment, value);
	}

	return res;
}