#pragma once

#include "Node.h"
#include "Token.h"
#include "TokenType.h"

MainNode* CreateAST(vector<Token> tokens);