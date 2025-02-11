/* ASTBreakStmt.h - AST node to represent a break statement.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#ifndef WEAK_COMPILER_FRONTEND_AST_AST_BREAK_H
#define WEAK_COMPILER_FRONTEND_AST_AST_BREAK_H

#include "FrontEnd/AST/ASTNode.h"

namespace weak {

class ASTBreak : public ASTNode {
public:
  ASTBreak(unsigned LineNo, unsigned ColumnNo);

  void Accept(ASTVisitor *) override;
};

} // namespace weak

#endif // WEAK_COMPILER_FRONTEND_AST_AST_BREAK_H