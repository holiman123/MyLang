#include <iostream>
#include <fstream>
#include <string>
#include "Token.h"
#include "TokenType.h"
#include "Node.h"
#include "testCompile.h"

using namespace std;

static string helpStr = "Commands:\n\tcompile - compiles .myl file to .exe or to arduino\n\thelp - show this message\n\nFlags:\n\t--file\t\t-f - path to file to compile\n\t--paltform\t-p - specify to which platform compile (windows/ arduino)\n\t--COMport\t-c - specify COM port to upload arduino sketch\n\t--board\t\t-b - specify arduino board type (Fully Qualified Board Name)\n\t--debug\t\t-d - do show debug information\n\t--start\t\t-s - do start program after compilation";

void replace_all(std::string& s, std::string const& toReplace, std::string const& replaceWith) {
    std::string buf;
    std::size_t pos = 0;
    std::size_t prevPos;

    // Reserves rough estimate of final size of string.
    buf.reserve(s.size());

    while (true) {
        prevPos = pos;
        pos = s.find(toReplace, pos);
        if (pos == std::string::npos)
            break;
        buf.append(s, prevPos, pos - prevPos);
        buf += replaceWith;
        pos += toReplace.size();
    }

    buf.append(s, prevPos, s.size() - prevPos);
    s.swap(buf);
}

string translateStringsToArduino(string input)
{
    replace_all(input, "(string)", "(String)");
    return input;
}


void initTokinizers()
{
    vector<string> posVars = { "int", "float", "bool", "string"};

    Identifier::endSymbols = " =+-/*|&^%,.!{}[]()'>\";:~\n\t\0";
    Keyword::posValues = { "void", "int", "float", "bool", "if", "func", "else", "string", "return", "import", "function", "pipe", "to"};
    Keyword::endSymbols = " =+-/*|&^%,.!{}[]()'>\";:~\n\t\0";
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

            if (currentToken.value == "pipe")
            {
                position++;
                if (tokens[position].type != TokenType::Identifier)
                    throw invalid_argument("Wrong syntax!");

                PipeNode* pipe = new PipeNode();

                IdentifierNode* initVarNode = new IdentifierNode(tokens[position].value);

                pipe->nodes.push_back(initVarNode);

                position++;
                while (tokens[position].value != "to")
                {
                    if (tokens[position].value == ">")
                    {
                        position++;
                        IdentifierNode* funcId = new IdentifierNode(tokens[position].value);
                        pipe->nodes.push_back(funcId);
                    }

                    position++;
                }
                position++;

                IdentifierNode* resVarNode = new IdentifierNode(tokens[position].value);
                pipe->nodes.push_back(resVarNode);

                res->nodes.push_back(pipe);
                position++;

                continue;
            }

            throw invalid_argument("Wrong syntax!");
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
string compileToCppArduino(string code)
{
    initTokinizers();
    vector<Token> tokens = Tokenize(code);

    MainNode* AST = CreateAST(tokens);

    string libs = AST->ImportLibs();
    string functions = AST->DefineFunctions();
    string cppCode = libs + functions + "\nvoid setup() { _setup(); }\nvoid loop() { _loop(); }";
    cppCode = translateStringsToArduino(cppCode);
    return cppCode;
}


static string readFile(string path)
{
    ifstream inputFileStream;
    inputFileStream.open(path);

    string allCode;
    if (inputFileStream.is_open())
    {
        string line;
        while (getline(inputFileStream, line))
        {
            allCode.append(line + "\n");
        }
    }

    inputFileStream.close();

    return allCode;
}

static string getFileName(string path)
{
    int indexOfSlash = path.find_last_of('\\', path.length());
    string file = path;
    if (indexOfSlash != -1)
        file = path.erase(0, indexOfSlash);
    int indexOfDot = path.find_last_of('.', path.length());
    string name = file.erase(indexOfDot, file.length());

    return name;
}

static void compileForWindows(string code, string fileName)
{
    ofstream outputCppFile;
    outputCppFile.open(fileName + ".cpp");

    outputCppFile << compileToCpp(code);

    outputCppFile.close();

    string resCommand = "compile.bat " + fileName + ".cpp";
    system(resCommand.c_str());

    remove((fileName + ".cpp").c_str());
    remove((fileName + ".obj").c_str());

    cout << "Compilation is done! .exe file created." << endl;
}
static void compileForArduino(string code, string fileName, string board, string port, string upload)
{
    ofstream outputCppFile;
    outputCppFile.open("ArduinoBuild\\ArduinoBuild.ino");

    outputCppFile << compileToCppArduino(code);

    outputCppFile.close();

    string resCommand = "arduino-cli compile " + board + " " + port + " " + upload + " " + "ArduinoBuild\\ArduinoBuild.ino";
    system(resCommand.c_str());
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        cout << helpStr << endl;
    }
    
    for (size_t i = 1; i < argc; i++)
    {
        string command = argv[i];
        string t = argv[i + 1];

        if (command == "compile")
        {
            string filePath = "";
            string platform = "windows";
            string COMport = "";
            string boardName = "-b arduino:avr:nano";
            bool doDebug = false;
            bool doStart = false;
            string doUpload = "";

            // Get flags
            i++;
            for (;i < argc; i++)
            {
                string flag = argv[i];
                if (flag == "--file" || flag == "-f")
                {
                    i++;
                    filePath = argv[i];
                }
                else if (flag == "--platform" || flag == "-p")
                {
                    i++;
                    platform = argv[i];
                }
                else if (flag == "--COMport" || flag == "-c")
                {
                    i++;
                    COMport = (argv[i]);
                    COMport = "-p" + COMport;
                }
                else if (flag == "--board" || flag == "-b")
                {
                    i++;
                    boardName = (argv[i]);
                    boardName = "-b" + boardName;
                }
                else if (flag == "--debug" || flag == "-d")
                {
                    doDebug = true;
                }
                else if (flag == "--start" || flag == "-s")
                {
                    doStart = true;
                }
                else if (flag == "--upload" || flag == "-u")
                {
                    doUpload = "-u -v";
                }
            } 

            string allCode = readFile(filePath);
            string fileName = getFileName(filePath);

            if (platform == "windows")
            {
                compileForWindows(allCode, fileName);

                if (doStart)
                {
                    if (!doDebug)
                        system("cls");
                    else
                        cout << "\t--== MyLang program start ==--" << endl;

                    system((fileName + ".exe").c_str());
                }
            }
            else if (platform == "arduino")
            {
                compileForArduino(allCode, fileName, boardName, COMport, doUpload);
            }
        }
        else if (command == "help")
        {
            cout << helpStr << endl;
        }
        else
        {
            cout << "Unknown command. Use help." << endl;
        }
    }

    //// Arg1 file to compile
    //string arg1 = argv[1];
    //string fileFormat = arg1.substr(arg1.length()-3, 3);
    //string fileName = arg1.substr(0, arg1.length() - 4);
    //if (fileFormat.compare("myl"))
    //    return 0;

    //ifstream inputFileStream;
    //inputFileStream.open(arg1);

    //string allCode;
    //if (inputFileStream.is_open())
    //{
    //    string line;
    //    while (getline(inputFileStream, line))
    //    {
    //        allCode.append(line + "\n");
    //    }
    //}

    //ofstream outputCppFile;
    //outputCppFile.open(fileName + ".cpp");

    //outputCppFile << compileToCpp(allCode);

    //outputCppFile.close();
    //inputFileStream.close();

    //string resCommand = "compile.bat " + fileName + ".cpp";
    //system(resCommand.c_str());
    //
    ////system("cls");
    //cout << "\t--== MyLang program start ==--" << endl;

    //system((fileName + ".exe").c_str());

    //getchar();
    return 1;
}