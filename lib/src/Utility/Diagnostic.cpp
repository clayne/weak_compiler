/* Diagnostic.cpp - Helper functions, used to emitting errors and warns.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "Utility/Diagnostic.hpp"
#include "FrontEnd/AST/ASTNode.hpp"

#include <iostream>
#include <sstream>

/// Forward declaration is in Diagnostic.hpp, so there is no unnamed namespace.
class Diagnostic {
public:
  enum DiagLevel { WARN, ERROR } const Level;

  Diagnostic(enum DiagLevel TheLevel) : Level(TheLevel) {}

  static void ClearErrBuf() { std::ostringstream().swap(ErrBuf); }

  void EmitLabel(unsigned LineNo, unsigned ColumnNo) {
    ErrBuf << ((Level == ERROR) ? "Error" : "Warning");
    ErrBuf << " at line " << LineNo << ", column " << ColumnNo << ": ";
  }

  void EmitEmptyLabel() {
    ErrBuf << ((Level == ERROR) ? "Error" : "Warning");
    ErrBuf << ": ";
  }

  static inline std::ostringstream ErrBuf;
};

void weak::UnreachablePoint(std::string Msg) {
  std::string ErrMsg = "Unreachable point reached: " + Msg + '\n';
  throw std::runtime_error(ErrMsg);
}

weak::OstreamRAII::~OstreamRAII() noexcept(false) {
  std::string Buf = DiagImpl->ErrBuf.str();

  if (DiagImpl->Level == Diagnostic::ERROR) {
    throw std::runtime_error(Buf);
  }

  std::cerr << Buf << std::endl;
}

std::ostream &weak::OstreamRAII::operator<<(const char *String) {
  return DiagImpl->ErrBuf << String;
}

static weak::OstreamRAII MakeMessage(Diagnostic::DiagLevel Level) {
  Diagnostic::ClearErrBuf();
  static Diagnostic Diag(Level);
  Diag.EmitEmptyLabel();
  return weak::OstreamRAII{&Diag};
}

static weak::OstreamRAII MakeMessage(Diagnostic::DiagLevel Level,
                                     unsigned LineNo, unsigned ColumnNo) {
  Diagnostic::ClearErrBuf();
  static Diagnostic Diag(Level);
  Diag.EmitLabel(LineNo, ColumnNo);
  return weak::OstreamRAII{&Diag};
}

weak::OstreamRAII weak::CompileWarning() {
  return MakeMessage(Diagnostic::WARN);
}

weak::OstreamRAII weak::CompileWarning(unsigned LineNo, unsigned ColumnNo) {
  return MakeMessage(Diagnostic::WARN, LineNo, ColumnNo);
}

weak::OstreamRAII weak::CompileError() {
  return MakeMessage(Diagnostic::ERROR);
}

weak::OstreamRAII weak::CompileError(unsigned LineNo, unsigned ColumnNo) {
  return MakeMessage(Diagnostic::ERROR, LineNo, ColumnNo);
}

weak::OstreamRAII weak::CompileError(const ASTNode *Node) {
  unsigned LineNo = Node->GetLineNo();
  unsigned ColumnNo = Node->GetColumnNo();
  return weak::CompileError(LineNo, ColumnNo);
}