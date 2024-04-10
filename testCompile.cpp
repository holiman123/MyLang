#include <iostream>
#include <fstream>
#include <string>
#include "Token.h"
#include "TokenType.h"
#include "Node.h"
#include "testCompile.h"

using namespace std;

void initTokinizers()
{
    vector<string> posVars = { "int", "float", "bool", "string"};

    Identifier::endSymbols = " =+-/*|&^%,.!{}[]()'\";:~\n\t\0";
    Keyword::posValues = { "void", "int", "float", "bool", "if", "func", "else", "string", "return", "import", "function"};
    Keyword::endSymbols = " =+-/*|&^%,.!{}[]()'\";:~\n\t\0";
    Keyword::posVars = posVars;
    Separator::posValues = { ";", ":", "(", ")", "{", "}", "[", "]", ".", ",", "\n", "\t"};
    // Longest OP's must be first
    Operator::posValues = { "not", "==", "!=", "+", "-", "*", "/", "=", ">", "<", "%", "^", "|", "&"};
    Literal::posValues = { "false", "true" };
    Comment::commentSymbol = "##";
}

vector<Token> Tokenize(string code)
{
    int position = 0;
    int codeLength = code.length();
    vector<Token> res;

    while (position <= codeLength)
    {
        Token token;
        bool flag = true;

        // Tokenize order:
        // 1) Skip whitespace
        // 2) Keyword
        // 3) Literal
        // 4) Separator
        // 5) Operator
        // 6) Comment
        // 7) Identifier

        // 1) Skip whitespace
        if (code[position] == ' ' || code[position] == '\0')
        {
            flag = false;
            position += 1;
        }

        // 2) Keyword
        if (flag)
        {
            token = Keyword::CheckAndCreate(code, position);
            if (token.value.length() != 0) flag = false;
        }

        // 3) Literal
        if (flag)
        {
            token = Literal::CheckAndCreate(code, position);
            if (token.value.length() != 0) flag = false;
        }

        // 4) Separator
        if (flag)
        {
            token = Separator::CheckAndCreate(code, position);
            if (token.value.length() != 0) flag = false;
        }

        // 5) Operator
        if (flag)
        {
            token = Operator::CheckAndCreate(code, position);
            if (token.value.length() != 0) flag = false;
        }

        // 6) Comment
        if (flag)
        {
            token = Comment::CheckAndCreate(code, position);
            if (token.value.length() != 0) flag = false;
        }

        // 7) Identifier
        if (flag)
        {
            token = Identifier::CheckAndCreate(code, position);
            if (token.value.length() != 0) flag = false;
        }        

        if (flag == true)
        {
            throw invalid_argument("Unknown token");
        }

        position += token.value.length();
        if(token.value.length() != 0)
            res.push_back(token);
    }

    return res;
}

MainNode* CreateAST(vector<Token> tokens)
{
    MainNode* res = new MainNode();

    int position = 0;
    while(position < tokens.size())
    {
        Token currentToken = tokens[position];

        // Keyword
        if (currentToken.type == TokenType::Keyword)
        {
            if (find(Keyword::posVars.begin(), Keyword::posVars.end(), currentToken.value) != Keyword::posVars.end())
                //tokens[position].value == "int" || tokens[position].value == "bool" || tokens[position].value == "string")
            {
                if (tokens[position + 1].type != TokenType::Identifier)
                    throw invalid_argument("Wrong syntax!");

                CreateVarNode* varNode = new CreateVarNode();
                DataTypeNode* type = new DataTypeNode(tokens[position].value);
                IdentifierNode* id = new IdentifierNode(tokens[position + 1].value);
                varNode->nodes.push_back(type);
                varNode->nodes.push_back(id);
                res->nodes.push_back(varNode);
                position += 2;

                if (tokens[position].value != "=")
                    continue;

                OperAssignNode* opAss = new OperAssignNode();

                position++;
                ExpressionNode* expNode = new ExpressionNode(tokens, &position);

                opAss->nodes.push_back(id);
                opAss->nodes.push_back(expNode);
                res->nodes.push_back(opAss);

                continue;
            }

            if (currentToken.value == "import")
            {
                if (tokens[position + 1].type != TokenType::Literal)
                    throw invalid_argument("Wrong syntax!");

                ImportNode* node = new ImportNode(tokens[position + 1].value.substr(1, tokens[position + 1].value.size() - 2));
                res->nodes.push_back(node);

                position += 2;
                continue;
            }

            if (currentToken.value == "function")
            {
                if (tokens[position + 1].type == TokenType::Keyword)
                {
                        FuncDefineNode* funcDefNode = new FuncDefineNode();

                        // Check and get return data type node
                        position++;
                        if (tokens[position].type != TokenType::Keyword)
                            throw invalid_argument("Wrong syntax!");

                        DataTypeNode* returnType = new DataTypeNode(tokens[position].value);
                        funcDefNode->nodes.push_back(returnType);
                        
                        // ID node
                        position++;
                        if (tokens[position].type != TokenType::Identifier)
                            throw invalid_argument("Wrong syntax!");

                        IdentifierNode* id = new IdentifierNode(tokens[position].value);
                        funcDefNode->nodes.push_back(id);

                        // Get params nodes
                        position++;
                        if (tokens[position].value != "(")
                            throw invalid_argument("Wrong syntax!");
                        
                        position++;
                        while (tokens[position].value != ")")
                        {
                            if (tokens[position].type == TokenType::Keyword)
                            {
                                if (tokens[position].value == "int")
                                {
                                    FuncArgNode* arg = new FuncArgNode();
                                    position++;
                                    if (tokens[position].type == TokenType::Identifier)
                                    {
                                        arg->nodes.push_back(new DataTypeNode("int"));
                                        arg->nodes.push_back(new IdentifierNode(tokens[position].value));
                                    }

                                    funcDefNode->nodes.push_back(arg);
                                }

                                if (tokens[position].value == "string")
                                {
                                    FuncArgNode* arg = new FuncArgNode();
                                    position++;
                                    if (tokens[position].type == TokenType::Identifier)
                                    {
                                        arg->nodes.push_back(new DataTypeNode("string"));
                                        arg->nodes.push_back(new IdentifierNode(tokens[position].value));
                                    }

                                    funcDefNode->nodes.push_back(arg);
                                }

                                if (tokens[position].value == "function")
                                {
                                    FuncFuncArgNode* arg = new FuncFuncArgNode();

                                    position++;

                                    // Return type
                                    arg->nodes.push_back(new DataTypeNode(tokens[position].value));
                                    position++;

                                    // ID
                                    arg->nodes.push_back(new IdentifierNode(tokens[position].value));
                                    position++;

                                    // Arg types
                                    if (tokens[position].type == Separator)
                                    {
                                        position++;
                                        while (tokens[position].type != Separator)
                                        {
                                            arg->nodes.push_back(new DataTypeNode(tokens[position].value));
                                            position++;
                                        }
                                    }

                                    funcDefNode->nodes.push_back(arg);
                                }
                            }

                            position++;
                        }

                        position++;
                        while (tokens[position].value == "\n" || tokens[position].value == "\t")
                            position++;

                        BodyNode* body = new BodyNode(tokens, &position);
                        funcDefNode->nodes.push_back(body);

                        res->nodes.push_back(funcDefNode);
                        continue;
                }
                else
                {
                    
                }
            }

            if (currentToken.value == "return")
            {
                FuncReturnNode* retNode = new FuncReturnNode();

                position++;
                ExpressionSingleNode* expNode = new ExpressionSingleNode(tokens, &position);

                retNode->nodes.push_back(expNode);

                res->nodes.push_back(retNode);

                continue;
            }

            if (currentToken.value == "if")
            {
                IfNode* ifNode = new IfNode();
                position++;
                ExpressionNode* condition = new ExpressionNode(tokens, &position);
                position++;
                BodyNode* body = new BodyNode(tokens, &position);

                ifNode->nodes.push_back(condition);
                ifNode->nodes.push_back(body);

                res->nodes.push_back(ifNode);

                continue;
            }

            throw invalid_argument("Wrong Syntax!");
        }

        // Identifier
        if (currentToken.type == TokenType::Identifier)
        {
            if (tokens[position + 1].type == TokenType::Operator)
            {
                position++;
                Token op = tokens[position];
                position++;
                if (op.value == "=")
                {
                    IdentifierNode* id = new IdentifierNode(currentToken.value);
                    ExpressionNode* expNode = new ExpressionNode(tokens, &position);
                    OperAssignNode* assNode = new OperAssignNode();
                    assNode->nodes.push_back(id);
                    assNode->nodes.push_back(expNode);
                    res->nodes.push_back(assNode);

                    continue;
                }
            }

            //continue;
        }

        // Expression
        if (currentToken.type == TokenType::Identifier && tokens[position + 1].type == TokenType::Separator)
        {
            ExpressionSingleNode* expNode = new ExpressionSingleNode(tokens, &position);
            res->nodes.push_back(expNode);
        }

        //throw invalid_argument("Wrong syntax!");
        position++;
    }

    return res;
}

string compileToCpp(string code)
{
    initTokinizers();
    vector<Token> tokens = Tokenize(code);
    
    MainNode* AST = CreateAST(tokens);
    string libs = AST->ImportLibs();
    string functions = AST->DefineFunctions();
    string body = AST->Parse();
    string cppCode = libs + functions + "int main() {\n" + body + "\nreturn 0; \n }";
    return cppCode;
}

int main(int argc, char *argv[])
{
    string arg1 = argv[1];
    string fileFormat = arg1.substr(arg1.length()-3, 3);
    string fileName = arg1.substr(0, arg1.length() - 4);
    if (fileFormat.compare("myl"))
        return 0;

    ifstream inputFileStream;
    inputFileStream.open(arg1);

    string allText;
    string line;
    if (inputFileStream.is_open())
    {
        while (getline(inputFileStream, line))
        {
            allText.append(line + "\n");
        }
    }

    string directory = argv[0];//program full path + name of binary file
    directory.erase(directory.find_last_of('\\') + 1);//remove name of binary file

    ofstream outputCppFile;
    outputCppFile.open(fileName + "Res.cpp");    

    outputCppFile << compileToCpp(allText);

    outputCppFile.close();
    inputFileStream.close();

    string resCommand = "compile.bat " + fileName + "Res.cpp";
    system(resCommand.c_str());    
    
    //system("cls");
    cout << "\t--== MyLang program start ==--" << endl;

    system((fileName + "Res.exe").c_str());

    getchar();
    return 1;
}