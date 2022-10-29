/* ASTDump.cpp - AST dumper.
 * Copyright (C) 2022 epoll-reactor <glibcxx.chrono@gmail.com>
 *
 * This file is distributed under the MIT license.
 */

#include "FrontEnd/AST/ASTDump.h"
#include "FrontEnd/AST/ASTArrayAccess.h"
#include "FrontEnd/AST/ASTArrayDecl.h"
#include "FrontEnd/AST/ASTBinaryOperator.h"
#include "FrontEnd/AST/ASTBooleanLiteral.h"
#include "FrontEnd/AST/ASTBreakStmt.h"
#include "FrontEnd/AST/ASTCharLiteral.h"
#include "FrontEnd/AST/ASTCompoundStmt.h"
#include "FrontEnd/AST/ASTContinueStmt.h"
#include "FrontEnd/AST/ASTDoWhileStmt.h"
#include "FrontEnd/AST/ASTFloatingPointLiteral.h"
#include "FrontEnd/AST/ASTForStmt.h"
#include "FrontEnd/AST/ASTFunctionCall.h"
#include "FrontEnd/AST/ASTFunctionDecl.h"
#include "FrontEnd/AST/ASTFunctionPrototype.h"
#include "FrontEnd/AST/ASTIfStmt.h"
#include "FrontEnd/AST/ASTIntegerLiteral.h"
#include "FrontEnd/AST/ASTNode.h"
#include "FrontEnd/AST/ASTReturnStmt.h"
#include "FrontEnd/AST/ASTStringLiteral.h"
#include "FrontEnd/AST/ASTStructDecl.h"
#include "FrontEnd/AST/ASTSymbol.h"
#include "FrontEnd/AST/ASTUnaryOperator.h"
#include "FrontEnd/AST/ASTVarDecl.h"
#include "FrontEnd/AST/ASTVisitor.h"
#include "FrontEnd/AST/ASTWhileStmt.h"

template <typename It>
static std::string IntegerRangeToString(It Begin, It End) {
  std::string Output;
  Output.reserve(std::distance(Begin, End));

  Output += "[";
  for (; Begin != End; ++Begin)
    Output += std::to_string(*Begin) + "][";

  if (!Output.empty()) {
    Output.pop_back();
    Output.pop_back();
  }

  Output += "]";

  return Output;
}

namespace weak {
namespace {

class ASTDumpVisitor : public ASTVisitor {
public:
  ASTDumpVisitor(ASTNode *TheRootNode, std::ostream &TheOutStream)
      : RootNode(TheRootNode), Indent(0U), OutStream(TheOutStream) {}

  void Dump() { RootNode->Accept(this); }

private:
  void Visit(const ASTArrayAccess *Decl) override {
    PrintWithTextPos("ArrayAccess", Decl, /*NewLineNeeded=*/false);

    OutStream << Decl->GetSymbolName() << std::endl;

    Indent += 2;
    PrintIndent();
    Decl->GetIndex()->Accept(this);
    Indent -= 2;
  }

  void Visit(const ASTArrayDecl *Decl) override {
    PrintWithTextPos("ArrayDecl", Decl, /*NewLineNeeded=*/false);

    const auto &ArityList = Decl->GetArityList();
    OutStream << TokenToString(Decl->GetDataType()) << " "
              << IntegerRangeToString(ArityList.cbegin(), ArityList.cend())
              << " ";
    OutStream << Decl->GetSymbolName() << std::endl;
  }

  void Visit(const ASTBinaryOperator *Binary) override {
    PrintWithTextPos("BinaryOperator", Binary, /*NewLineNeeded=*/false);
    OutStream << TokenToString(Binary->GetOperation()) << std::endl;
    Indent += 2;

    PrintIndent();
    Binary->GetLHS()->Accept(this);

    PrintIndent();
    Binary->GetRHS()->Accept(this);

    Indent -= 2;
  }

  void Visit(const ASTBooleanLiteral *Boolean) override {
    PrintWithTextPos("BooleanLiteral", Boolean, /*NewLineNeeded=*/false);
    OutStream << std::boolalpha << Boolean->GetValue() << std::endl;
  }

  void Visit(const ASTBreakStmt *BreakStmt) override {
    PrintWithTextPos("BreakStmt", BreakStmt, /*NewLineNeeded=*/true);
  }

  void Visit(const ASTCharLiteral *Char) override {
    PrintWithTextPos("CharLiteral", Char, /*NewLineNeeded=*/false);
    OutStream << "'" << Char->GetValue() << "'" << std::endl;
  }

  void Visit(const ASTCompoundStmt *CompoundStmt) override {
    PrintWithTextPos("CompoundStmt", CompoundStmt, /*NewLineNeeded=*/true);

    Indent += 2;
    for (const auto &Stmt : CompoundStmt->GetStmts()) {
      PrintIndent();
      Stmt->Accept(this);
    }
    Indent -= 2;
  }

  void Visit(const ASTContinueStmt *ContinueStmt) override {
    PrintWithTextPos("ContinueStmt", ContinueStmt, /*NewLineNeeded=*/true);
  }

  void Visit(const ASTFloatingPointLiteral *Float) override {
    PrintWithTextPos("FloatingPointLiteral", Float,
                     /*NewLineNeeded=*/false);
    OutStream << Float->GetValue() << std::endl;
  }

  void Visit(const ASTForStmt *ForStmt) override {
    PrintWithTextPos("ForStmt", ForStmt,
                     /*NewLineNeeded=*/true);

    Indent += 2;

    if (auto *Init = ForStmt->GetInit()) {
      PrintIndent();
      PrintWithTextPos("ForStmtInit", Init,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Init->Accept(this);
      Indent -= 2;
    }

    if (auto *Condition = ForStmt->GetCondition()) {
      PrintIndent();
      PrintWithTextPos("ForStmtCondition", Condition,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Condition->Accept(this);
      Indent -= 2;
    }

    if (auto *Increment = ForStmt->GetIncrement()) {
      PrintIndent();
      PrintWithTextPos("ForStmtIncrement", Increment,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Increment->Accept(this);
      Indent -= 2;
    }

    if (const auto *Body = ForStmt->GetBody()) {
      PrintIndent();
      PrintWithTextPos("ForStmtBody", Body,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Visit(Body);
      Indent -= 2;
    }

    Indent -= 2;
  }

  void Visit(const ASTIfStmt *IfStmt) override {
    PrintWithTextPos("IfStmt", IfStmt, /*NewLineNeeded=*/true);
    Indent += 2;

    if (const auto &Condition = IfStmt->GetCondition()) {
      PrintIndent();
      PrintWithTextPos("IfStmtCondition", Condition,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Condition->Accept(this);
      Indent -= 2;
    }

    if (const auto &ThenBody = IfStmt->GetThenBody()) {
      PrintIndent();
      PrintWithTextPos("IfStmtThenBody", ThenBody,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Visit(ThenBody);
      Indent -= 2;
    }

    if (const auto &ElseBody = IfStmt->GetElseBody()) {
      PrintIndent();
      PrintWithTextPos("IfStmtElseBody", ElseBody,
                       /*NewLineNeeded=*/true);
      Indent += 2;
      PrintIndent();
      Visit(ElseBody);
      Indent -= 2;
    }
    Indent -= 2;
  }

  void Visit(const ASTIntegerLiteral *Integer) override {
    PrintWithTextPos("IntegerLiteral", Integer,
                     /*NewLineNeeded=*/false);
    OutStream << Integer->GetValue() << std::endl;
  }

  void Visit(const ASTReturnStmt *ReturnStmt) override {
    PrintWithTextPos("ReturnStmt", ReturnStmt, /*NewLineNeeded=*/true);
    Indent += 2;

    if (ReturnStmt->GetOperand()) {
      PrintIndent();
      ReturnStmt->GetOperand()->Accept(this);
    }

    Indent -= 2;
  }

  void Visit(const ASTStringLiteral *String) override {
    PrintWithTextPos("StringLiteral", String,
                     /*NewLineNeeded=*/false);
    OutStream << String->GetValue() << std::endl;
  }

  void Visit(const ASTSymbol *Symbol) override {
    PrintWithTextPos("Symbol", Symbol,
                     /*NewLineNeeded=*/false);
    OutStream << Symbol->GetName() << std::endl;
  }

  void Visit(const ASTUnaryOperator *Unary) override {
    OutStream << (Unary->PrefixOrPostfix == ASTUnaryOperator::PREFIX
                      ? "Prefix "
                      : "Postfix ");
    PrintWithTextPos("UnaryOperator", Unary,
                     /*NewLineNeeded=*/false);
    OutStream << TokenToString(Unary->GetOperation()) << std::endl;
    Indent += 2;

    PrintIndent();
    Unary->GetOperand()->Accept(this);

    Indent -= 2;
  }

  void Visit(const ASTStructDecl *Decl) override {
    PrintWithTextPos("StructDecl", Decl, /*NewLineNeeded=*/false);
    OutStream << Decl->GetName() << std::endl;

    Indent += 2;
    for (const auto &Field : Decl->GetDecls()) {
      PrintIndent();
      Field->Accept(this);
    }
    Indent -= 2;
  }

  void Visit(const ASTVarDecl *VarDecl) override {
    PrintWithTextPos("VarDecl", VarDecl, /*NewLineNeeded=*/false);
    OutStream << TokenToString(VarDecl->GetDataType()) << " "
              << VarDecl->GetSymbolName() << std::endl;

    if (const auto &Body = VarDecl->GetDeclBody()) {
      Indent += 2;
      PrintIndent();
      Body->Accept(this);
      Indent -= 2;
    }
  }

  void Visit(const ASTFunctionDecl *FunctionDecl) override {
    PrintWithTextPos("FunctionDecl", FunctionDecl, /*NewLineNeeded=*/true);

    Indent += 2;
    PrintIndent();
    PrintWithTextPos("FunctionDeclRetType", FunctionDecl,
                     /*NewLineNeeded=*/false);
    OutStream << TokenToString(FunctionDecl->GetReturnType()) << std::endl;

    PrintIndent();
    PrintWithTextPos("FunctionDeclName", FunctionDecl,
                     /*NewLineNeeded=*/false);
    OutStream << FunctionDecl->GetName() << std::endl;

    PrintIndent();
    PrintWithTextPos("FunctionDeclArgs", FunctionDecl,
                     /*NewLineNeeded=*/true);

    Indent += 2;
    for (const auto &Argument : FunctionDecl->GetArguments()) {
      PrintIndent();
      Argument->Accept(this);
    }
    Indent -= 2;

    PrintIndent();
    PrintWithTextPos("FunctionDeclBody", FunctionDecl,
                     /*NewLineNeeded=*/true);

    Indent += 2;
    PrintIndent();
    Visit(FunctionDecl->GetBody());
    Indent -= 2;
  }

  void Visit(const ASTFunctionCall *FunctionCall) override {
    PrintWithTextPos("FunctionCall", FunctionCall,
                     /*NewLineNeeded=*/false);
    OutStream << FunctionCall->GetName() << std::endl;

    Indent += 2;
    PrintIndent();
    PrintWithTextPos("FunctionCallArgs", FunctionCall,
                     /*NewLineNeeded=*/true);

    Indent += 2;
    for (const auto &Argument : FunctionCall->GetArguments()) {
      PrintIndent();
      Argument->Accept(this);
    }
    Indent -= 2;

    Indent -= 2;
  }

  void Visit(const ASTFunctionPrototype *FunctionPrototype) override {
    PrintWithTextPos("FunctionPrototype", FunctionPrototype,
                     /*NewLineNeeded=*/false);
    OutStream << FunctionPrototype->GetName() << std::endl;

    Indent += 2;
    PrintIndent();
    PrintWithTextPos("FunctionPrototypeArgs", FunctionPrototype,
                     /*NewLineNeeded=*/true);

    Indent += 2;
    for (const auto &Argument : FunctionPrototype->GetArguments()) {
      PrintIndent();
      Argument->Accept(this);
    }
    Indent -= 2;

    Indent -= 2;
  }

  void Visit(const ASTDoWhileStmt *DoWhileStmt) override {
    CommonWhileStmtVisit(DoWhileStmt, /*IsDoWhile=*/true);
  }

  void Visit(const ASTWhileStmt *WhileStmt) override {
    CommonWhileStmtVisit(WhileStmt, /*IsDoWhile=*/false);
  }

  template <typename WhileNode>
  void CommonWhileStmtVisit(const WhileNode *WhileStmt, bool IsDoWhile) {
    using namespace std::string_literals;
    PrintWithTextPos((IsDoWhile ? "Do"s : ""s) + "WhileStmt", WhileStmt,
                     /*NewLineNeeded=*/true);

    auto PrintWhileCondition = [&] {
      if (const auto &Condition = WhileStmt->GetCondition()) {
        PrintIndent();
        PrintWithTextPos((IsDoWhile ? "Do"s : ""s) + "WhileStmtCond", Condition,
                         /*NewLineNeeded=*/true);
        Indent += 2;
        PrintIndent();
        Condition->Accept(this);
        Indent -= 2;
      }
    };

    auto PrintWhileBody = [&] {
      if (const auto &Body = WhileStmt->GetBody()) {
        PrintIndent();
        PrintWithTextPos((IsDoWhile ? "Do"s : ""s) + "WhileStmtBody", Body,
                         /*NewLineNeeded=*/true);
        Indent += 2;
        PrintIndent();
        Visit(Body);
        Indent -= 2;
      }
    };

    Indent += 2;

    if (IsDoWhile) {
      PrintWhileBody();
      PrintWhileCondition();
    } else {
      PrintWhileCondition();
      PrintWhileBody();
    }

    Indent -= 2;
  }

  void PrintWithTextPos(std::string_view Label, const ASTNode *Node,
                        bool NewLineNeeded) const {
    OutStream << Label << " <line:" << Node->GetLineNo()
              << ", col:" << Node->GetColumnNo() << ">";

    if (NewLineNeeded)
      OutStream << std::endl;
    else
      OutStream << " ";
  }

  void PrintIndent() const { OutStream << std::string(Indent, ' '); }

  ASTNode *RootNode;
  unsigned Indent;
  std::ostream &OutStream;
};

} // namespace
} // namespace weak

void weak::ASTDump(ASTNode *RootNode, std::ostream &OutStream) {
  ASTDumpVisitor DumpVisitor(RootNode, OutStream);
  DumpVisitor.Dump();
}