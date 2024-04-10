#pragma once

//#include <vector>
#include <string>
#include <list>
#include <vector>
#include "Token.h"

using namespace std;

class Node
{
public:
	vector<Node*> nodes;
	virtual string Parse();
};

class MainNode : public Node
{
public:
	string ImportLibs();
	string DefineFunctions();
	using Node::Parse;
	string Parse() override;
};

class ImportNode : public Node
{
private:
	string path;
public:
	ImportNode(string path);
	using Node::Parse;
	string Parse() override;
};

class FuncCallNode : public Node
{
public:
	FuncCallNode(vector<Token> tokens, int position);
	using Node::Parse;
	string Parse() override;
};

class CreateVarNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};

class IdentifierNode : public Node
{
private:
	string name;
public:
	IdentifierNode(string name);
	using Node::Parse;
	string Parse() override;
};

class OperAssignNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};

class OperNotNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};

class NumberNode : public Node
{
private:
	string number;
public:
	NumberNode(string number);
	using Node::Parse;
	string Parse() override;
};

class StringNode : public Node
{
private:
	string text;
public:
	StringNode(string text);
	using Node::Parse;
	string Parse() override;
};

class ExpressionNode : public Node
{
public:
	ExpressionNode(vector<Token> tokens, int* position);
	using Node::Parse;
	string Parse() override;
};

class ExpressionSingleNode : public ExpressionNode
{
public:
	ExpressionSingleNode(vector<Token> tokens, int* position);
	using ExpressionNode::Parse;
	string Parse() override;
};

class DataTypeNode : public Node
{
private:
	string type;
public:
	DataTypeNode(string type);
	using Node::Parse;
	string Parse() override;
};

class FuncVarNode : public Node
{
private:
	string valueName;
public:
	FuncVarNode(string funcName);
	using Node::Parse;
	string Parse() override;
};

class FuncDefineNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};

class BodyNode : public Node
{
public:
	BodyNode(vector<Token> tokens, int* position);
	using Node::Parse;
	string Parse() override;
};

class FuncReturnNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};

class FuncArgNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};
class FuncFuncArgNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};

class IfNode : public Node
{
public:
	using Node::Parse;
	string Parse() override;
};