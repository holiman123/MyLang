#include "Node.h"
#include <iostream>
#include <string>
#include <fstream>
#include "Token.h"
#include "testCompile.h"

using namespace std;

static bool isArduino = false;

string Node::Parse()
{
	return "";
}

string MainNode::Parse()
{
	string cppCode = "";
	for (Node* node : nodes)
	{

		if (dynamic_cast<ImportNode*> (node) == NULL && dynamic_cast<FuncDefineNode*> (node) == NULL)
		{
			cppCode += node->Parse();
			cppCode += "\n";
		}
	}
	return cppCode;
}
string MainNode::ImportLibs()
{
	string cppCode = "";
	for (Node* node : nodes)
	{
		if (dynamic_cast<ImportNode*> (node) == NULL)
			break;

		cppCode += node->Parse() + "\n";
	}

	return cppCode;
}
string MainNode::DefineFunctions()
{
	string cppCode = "";
	for (Node* node : nodes)
	{
		if (dynamic_cast<FuncDefineNode*> (node) == NULL)
			continue;

		cppCode += node->Parse() + "\n";
	}

	return cppCode;
}

ImportNode::ImportNode(string path)
{
	this->path = path;
}
string ImportNode::Parse()
{
	// Load function from file
	string fileFormat = path.substr(path.length() - 6, 6);
	string fileName = path.substr(0, path.length() - 7);
	if (fileFormat.compare("myllib"))
		throw invalid_argument("Invalid library file format");

	ifstream inputFileStream;
	inputFileStream.open(path);

	string allText;
	string firstLine;
	getline(inputFileStream, firstLine);
	string line;
	if (inputFileStream.is_open())
	{
		while (getline(inputFileStream, line))
		{
			allText.append(line + "\n");
		}
	}

	if (firstLine == "cpp")
	{
		return allText;
	}

	// TODO: implement myl libriry load
	if (firstLine == "myl")
	{
		throw invalid_argument("not implement now");
		return "";
	}

	throw invalid_argument("Unknown library format");
	return "";
}

FuncCallNode::FuncCallNode(vector<Token> tokens, int position)
{
	if (tokens[position + 1].value == "(")
	{
		nodes.push_back(new IdentifierNode(tokens[position].value));
		position += 2;
		while (tokens[position].value != ")")
		{
			// Identifier as parameter
			if (tokens[position].type == TokenType::Identifier && tokens[position + 1].value != "(")
			{
				nodes.push_back(new IdentifierNode(tokens[position].value));
			}

			// Literal as parameter
			if (tokens[position].type == TokenType::Literal)
			{
				if (tokens[position].value[0] == '\"')
					nodes.push_back(new StringNode(tokens[position].value));
				else
					nodes.push_back(new NumberNode(tokens[position].value));
			}

			// Function as parameter
			if (tokens[position].type == TokenType::Identifier && tokens[position + 1].value == "(")
			{
				FuncCallNode* innerFunc = new FuncCallNode(tokens, position);
				nodes.push_back(innerFunc);

				int posCounter = 0;
				for (size_t i = position; i < tokens.size(); i++)
				{
					if (tokens[i].value != ")")
						posCounter++;
					else
						break;
				}

				position += posCounter;
			}

			position++;
		}
	}
}
string FuncCallNode::Parse()
{
	string cppCode = "";
	cppCode += nodes[0]->Parse() + "(";

	for (size_t i = 1; i < nodes.size(); i++)
	{
		cppCode += nodes[i]->Parse();
		if (i != nodes.size() - 1)
			cppCode += ", ";
	}
	
	cppCode += ")";

	return cppCode;
}

string CreateVarNode::Parse()
{
	string cppCode = "";
	cppCode += nodes[0]->Parse(); // parse type
	cppCode += " ";
	cppCode += nodes[1]->Parse(); // parse var name
	cppCode += ";";
	return cppCode;
}

IdentifierNode::IdentifierNode(string name)
{
	this->name = name;
}
string IdentifierNode::Parse()
{
	return "_" + name;
}

string OperAssignNode::Parse()
{
	string cppCode = "";
	cppCode += nodes[0]->Parse() + " = " + nodes[1]->Parse() + ";";
	return cppCode;
}

string OperNotNode::Parse()
{
	string cppCode = "!(";
	cppCode += nodes[0]->Parse();
	cppCode += ")";
	return cppCode;
}

NumberNode::NumberNode(string number)
{
	this->number = number;
}
string NumberNode::Parse()
{
	return number;
}

StringNode::StringNode(string text)
{
	this->text = text;
}
string StringNode::Parse()
{
	return "(string)" + text;
}

ExpressionNode::ExpressionNode(vector<Token> tokens, int* position)
{
	vector<Token> expTokens;
	//*position += 1;
	while (*position < tokens.size() && (!(tokens[*position].type == TokenType::Separator && tokens[*position].value == "\n")))
	{
		expTokens.push_back(tokens[*position]);
		*position += 1;
	}
	
	if (expTokens.size() == 1)
	{
		if (expTokens[0].type == TokenType::Literal)
		{
			if (expTokens[0].value[0] == '\"')
				nodes.push_back(new StringNode(expTokens[0].value));
			else
				nodes.push_back(new NumberNode(expTokens[0].value));

			return;
		}

		if (expTokens[0].type == TokenType::Identifier)
		{
			nodes.push_back(new IdentifierNode(expTokens[0].value));

			return;
		}
	}

	// Function
	if (expTokens[0].type == TokenType::Identifier && expTokens[1].type == TokenType::Separator)
	{
		FuncCallNode* funcCallNode = new FuncCallNode(expTokens, 0);

		nodes.push_back(funcCallNode);

		return;
	}

	// Operator
	if (expTokens[0].type == TokenType::Operator)
	{
		if (expTokens[0].value == "not")
		{
			OperNotNode* notNode = new OperNotNode();
			int localPos = 1;
			notNode->nodes.push_back(new ExpressionNode(expTokens, &localPos));
			position += localPos;

			nodes.push_back(notNode);

			return;
		}
	}

	throw invalid_argument("Unexpected token in expression");
}
string ExpressionNode::Parse()
{
	string cppCode = "";
	for (Node* node : nodes)
	{
		cppCode += node->Parse();
	}
	return cppCode;
}

ExpressionSingleNode::ExpressionSingleNode(vector<Token> tokens, int* pos) : ExpressionNode::ExpressionNode(tokens, pos) { }
string ExpressionSingleNode::Parse()
{
	return ExpressionNode::Parse() + ";";
}

DataTypeNode::DataTypeNode(string type)
{
	this->type = type;
}
string DataTypeNode::Parse()
{
	return type;
}



// returnType, ID, params, ... params, body - nodes order
string FuncDefineNode::Parse()
{
	string cppCode = "";
	cppCode += nodes[0]->Parse(); // return type
	cppCode += " ";
	cppCode += nodes[1]->Parse(); // name
	cppCode += "(";
	for (size_t i = 2; i < nodes.size() - 1; i++)
	{
		cppCode += nodes[i]->Parse();
		if (i != nodes.size() - 2)
			cppCode += ", ";
	}
	cppCode += ")";
	cppCode += nodes[nodes.size() - 1]->Parse();
	return cppCode;
}

BodyNode::BodyNode(vector<Token> tokens, int* position)
{
	while (tokens[*position].value != "{")
		*position += 1;

	if (tokens[*position].type == TokenType::Separator && tokens[*position].value == "{")
	{
		vector<Token> bodyTokens;
		*position += 1;
		int brCount = 1;
		while (brCount != 0)
		{
			if (tokens[*position].value == "{")
				brCount++;
			if (tokens[*position].value == "}")
				brCount--;

			bodyTokens.push_back(tokens[*position]);

			*position += 1;
		}

		MainNode* temp = CreateAST(bodyTokens);

		nodes = temp->nodes;
	}
}
string BodyNode::Parse()
{
	string cppCode = "\n{\n";
	for (Node* node : nodes)
	{
		cppCode += "\t" + node->Parse();
	}
	cppCode += "\n}\n";
	return cppCode;
}

string FuncReturnNode::Parse()
{
	string cppCode = "return ";

	cppCode += nodes[0]->Parse();

	return cppCode;
}

string FuncArgNode::Parse()
{
	string cppCode = "";

	cppCode += nodes[0]->Parse();
	cppCode += " ";
	cppCode += nodes[1]->Parse();

	return cppCode;
}

string FuncFuncArgNode::Parse()
{
	string cppCode = "";

	cppCode += nodes[0]->Parse();
	cppCode += "(*" + nodes[1]->Parse() + ")";
	cppCode += "(";
	for (size_t i = 2; i < nodes.size(); i++)
	{
		cppCode += nodes[i]->Parse();
		if (i != nodes.size() - 1)
			cppCode += ",";
	}
	cppCode += ")";

	return cppCode;
}

// Condition, body
string IfNode::Parse()
{
	string cppCode = "if (";
	cppCode += nodes[0]->Parse();
	cppCode += ")";
	cppCode += nodes[1]->Parse();
	return cppCode;
}

FuncVarNode::FuncVarNode(string funcName)
{
	valueName = funcName;
}
string FuncVarNode::Parse()
{
	return valueName;
}

// InitVar, functions nodes, resVar
string PipeNode::Parse()
{
	vector<Node*> reversed;

	for (int i = nodes.size() - 1; i >= 0; i--)
		reversed.push_back(nodes[i]);

	string cppCode = "";
	cppCode += reversed[0]->Parse();
	cppCode += " = ";
	for (int i = 1; i < reversed.size() - 1; i++)
	{
		cppCode += reversed[i]->Parse();
		cppCode += "(";
	}
	cppCode += nodes[0]->Parse();
	for (int i = 1; i < reversed.size() - 1; i++)
	{
		cppCode += ")";
	}
	cppCode += ";\n";

	return cppCode;
}