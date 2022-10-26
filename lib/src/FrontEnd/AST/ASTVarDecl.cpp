/* ASTVarDecl.cpp - AST node to represent a variable declaration.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "FrontEnd/AST/ASTVarDecl.hpp"
#include "FrontEnd/AST/ASTVisitor.hpp"

namespace weak {

ASTVarDecl::ASTVarDecl(TokenType TheDataType, std::string &&TheSymbolName,
                       ASTNode *TheDeclBody, unsigned TheLineNo,
                       unsigned TheColumnNo)
    : ASTNode(TheLineNo, TheColumnNo), DataType(TheDataType),
      SymbolName(std::move(TheSymbolName)), DeclBody(TheDeclBody) {}

ASTVarDecl::~ASTVarDecl() { delete DeclBody; }

ASTType ASTVarDecl::GetASTType() const { return AST_VAR_DECL; }

void ASTVarDecl::Accept(ASTVisitor *Visitor) { Visitor->Visit(this); }

TokenType ASTVarDecl::GetDataType() const { return DataType; }

const std::string &ASTVarDecl::GetSymbolName() const { return SymbolName; }

ASTNode *ASTVarDecl::GetDeclBody() const { return DeclBody; }

} // namespace weak