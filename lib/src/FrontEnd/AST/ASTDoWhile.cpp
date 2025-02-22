/* ASTDoWhile.cpp - AST node to represent a do-while statement.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "FrontEnd/AST/ASTDoWhile.h"
#include "FrontEnd/AST/ASTVisitor.h"

namespace weak {

ASTDoWhile::ASTDoWhile(
  ASTCompound *Body,
  ASTNode     *Condition,
  unsigned     LineNo,
  unsigned     ColumnNo
) : ASTNode(AST_DO_WHILE_STMT, LineNo, ColumnNo)
  , mBody(Body)
  , mCondition(Condition) {}

ASTDoWhile::~ASTDoWhile() {
  delete mBody;
  delete mCondition;
}

void ASTDoWhile::Accept(ASTVisitor *Visitor) {
  Visitor->Visit(this);
}

ASTCompound *ASTDoWhile::Body() const {
  return std::move(mBody);
}

ASTNode *ASTDoWhile::Condition() const {
  return mCondition;
}

} // namespace weak