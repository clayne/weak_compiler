#include "FrontEnd/Lex/Lexer.hpp"
#include "Utility/Diagnostic.hpp"
#include <cassert>
#include <unordered_map>

using TokenType = weak::frontEnd::TokenType;

static const std::unordered_map<std::string_view, TokenType> LexKeywords = {
    {"bool", TokenType::BOOLEAN},  {"break", TokenType::BREAK},
    {"char", TokenType::CHAR},     {"continue", TokenType::CONTINUE},
    {"do", TokenType::DO},         {"false", TokenType::FALSE},
    {"for", TokenType::FOR},       {"if", TokenType::IF},
    {"int", TokenType::INT},       {"return", TokenType::RETURN},
    {"string", TokenType::STRING}, {"true", TokenType::TRUE},
    {"void", TokenType::VOID},     {"while", TokenType::WHILE}};

static const std::unordered_map<std::string_view, TokenType> LexOperators = {
    {"=", TokenType::ASSIGN},
    {"*=", TokenType::MUL_ASSIGN},
    {"/=", TokenType::DIV_ASSIGN},
    {"%=", TokenType::MOD_ASSIGN},
    {"+=", TokenType::PLUS_ASSIGN},
    {"-=", TokenType::MINUS_ASSIGN},
    {"<<=", TokenType::SHL_ASSIGN},
    {">>=", TokenType::SHR_ASSIGN},
    {"&=", TokenType::BIT_AND_ASSIGN},
    {"|=", TokenType::BIT_OR_ASSIGN},
    {"^=", TokenType::XOR_ASSIGN},
    {"&&", TokenType::AND},
    {"||", TokenType::OR},
    {"^", TokenType::XOR},
    {"&", TokenType::BIT_AND},
    {"|", TokenType::BIT_OR},
    {"==", TokenType::EQ},
    {"!=", TokenType::NEQ},
    {">", TokenType::GT},
    {"<", TokenType::LT},
    {">=", TokenType::GE},
    {"<=", TokenType::LE},
    {">>", TokenType::SHR},
    {"<<", TokenType::SHL},
    {"+", TokenType::PLUS},
    {"-", TokenType::MINUS},
    {"*", TokenType::STAR},
    {"/", TokenType::SLASH},
    {"%", TokenType::MOD},
    {"++", TokenType::INC},
    {"--", TokenType::DEC},
    {",", TokenType::COMMA},
    {";", TokenType::SEMICOLON},
    {"!", TokenType::NOT},
    {"[", TokenType::OPEN_BOX_BRACKET},
    {"]", TokenType::CLOSE_BOX_BRACKET},
    {"{", TokenType::OPEN_CURLY_BRACKET},
    {"}", TokenType::CLOSE_CURLY_BRACKET},
    {"(", TokenType::OPEN_PAREN},
    {")", TokenType::CLOSE_PAREN}};

namespace {

class LexStringLiteralCheck {
public:
  explicit LexStringLiteralCheck(char ThePeek) : Peek(ThePeek) {}

  void ClosingQuoteCheck(unsigned LineNo, unsigned ColumnNo) const {
    if (Peek == '\n' || Peek == '\0') {
      weak::DiagnosticError(LineNo, ColumnNo) << "Closing \" expected";
    }
  }

private:
  char Peek;
};

class LexDigitCheck {
public:
  LexDigitCheck(const std::string &TheDigit, char ThePeek, unsigned Dots)
      : Digit(TheDigit), Peek(ThePeek), DotsReached(Dots) {}

  void LastDigitRequire(unsigned LineNo, unsigned ColumnNo) const {
    if (std::isalpha(Peek) || !std::isdigit(Digit.back())) {
      weak::DiagnosticError(LineNo, ColumnNo)
          << "Digit as last character expected";
    }
  }

  void ExactOneDotRequire(unsigned LineNo, unsigned ColumnNo) const {
    if (DotsReached > 1) {
      weak::DiagnosticError(LineNo, ColumnNo) << "Extra \".\" in digit";
    }
  }

private:
  const std::string &Digit;
  char Peek;
  unsigned DotsReached;
};

} // namespace

static bool IsAlphanumeric(char C) { return isalpha(C) || C == '_'; }

namespace weak {
namespace frontEnd {

Lexer::Lexer(const char *TheBufferStart, const char *TheBufferEnd)
    : BufferStart(TheBufferStart), BufferEnd(TheBufferEnd),
      CurrentBufferPtr(TheBufferStart), CurrentLineNo(0U), CurrentColumnNo(0U) {
  assert(BufferStart <= BufferEnd);
}

std::vector<Token> Lexer::Analyze() {
  std::vector<Token> ProcessedTokens;

  while (CurrentBufferPtr != BufferEnd) {
    if (char Atom = PeekCurrent(); std::isdigit(Atom)) {
      ProcessedTokens.push_back(AnalyzeDigit());
    } else if (std::isalpha(Atom)) {
      ProcessedTokens.push_back(AnalyzeSymbol());
    } else if (Atom == '\"') {
      ProcessedTokens.push_back(AnalyzeStringLiteral());
    } else if (std::isspace(Atom)) {
      PeekNext();
      continue;
    } else {
      ProcessedTokens.push_back(AnalyzeOperator());
    }
  }

  return ProcessedTokens;
}

Token Lexer::AnalyzeDigit() {
  std::string Digit;
  bool DotErrorOccurred = false;
  unsigned DotErrorColumn = 0;
  unsigned DotsReached = 0;

  while (std::isdigit(PeekCurrent()) || PeekCurrent() == '.') {
    if (PeekCurrent() == '.') {
      ++DotsReached;
    }
    if (DotsReached > 1) {
      DotErrorOccurred = true;
      DotErrorColumn = CurrentColumnNo;
      break;
    }
    Digit += PeekNext();
  }

  unsigned LexColumnName = DotErrorOccurred ? DotErrorColumn : CurrentColumnNo;

  LexDigitCheck Checker(Digit, PeekCurrent(), DotsReached);
  Checker.LastDigitRequire(CurrentLineNo, LexColumnName);
  Checker.ExactOneDotRequire(CurrentLineNo, LexColumnName);

  return MakeToken(Digit, DotsReached == 0 ? TokenType::INTEGRAL_LITERAL
                                           : TokenType::FLOATING_POINT_LITERAL);
}

Token Lexer::AnalyzeStringLiteral() {
  PeekNext(); // Eat "

  if (PeekNext() == '\"') {
    return MakeToken("", TokenType::STRING_LITERAL);
  }
  --CurrentBufferPtr;

  std::string Literal;
  while (PeekCurrent() != '\"') {
    Literal += PeekNext();

    LexStringLiteralCheck Check(PeekCurrent());
    Check.ClosingQuoteCheck(CurrentLineNo, CurrentColumnNo);

    if (Literal.back() == '\\') {
      Literal.back() = PeekNext();
    }
  }
  assert(PeekCurrent() == '\"');

  PeekNext(); // Eat "

  return MakeToken(Literal, TokenType::STRING_LITERAL);
}

Token Lexer::AnalyzeSymbol() {
  std::string Symbol;

  while ((IsAlphanumeric(PeekCurrent()) || std::isdigit(PeekCurrent()))) {
    Symbol += PeekNext();
  }

  if (LexKeywords.find(Symbol) != LexKeywords.end()) {
    return MakeToken("", LexKeywords.at(Symbol));
  } else {
    return MakeToken(std::move(Symbol), TokenType::SYMBOL);
  }
}

Token Lexer::AnalyzeOperator() {
  std::string Operator(1, PeekNext());
  unsigned SavedColumnNo = 0;

  while (LexOperators.find(Operator) != LexOperators.end()) {
    char Next = *CurrentBufferPtr++;
    SavedColumnNo = CurrentColumnNo;
    if (Next == '\n') {
      CurrentColumnNo = 0;
      CurrentLineNo++;
    }
    ++CurrentColumnNo;
    Operator += Next;

    if (LexOperators.find(Operator) == LexOperators.end()) {
      Operator.pop_back();
      --CurrentBufferPtr;
      if (CurrentColumnNo > 0) {
        --CurrentColumnNo;
      }
      if (PeekCurrent() == '\n') {
        --CurrentLineNo;
      }
      break;
    }
  }

  if (LexOperators.find(Operator) != LexOperators.end()) {
    return Token("", LexOperators.at(Operator), CurrentLineNo, SavedColumnNo);
  } else {
    --CurrentColumnNo;
    DiagnosticError(CurrentLineNo, CurrentColumnNo)
        << "Unknown character sequence " << Operator;
    /// Suppress warning about missing return statement.
    UnreachablePoint();
  }
}

char Lexer::PeekNext() {
  char Atom = *CurrentBufferPtr++;
  if (Atom == '\n') {
    CurrentLineNo++;
    CurrentColumnNo = 1U;
  } else {
    CurrentColumnNo++;
  }
  return Atom;
}

char Lexer::PeekCurrent() const { return *CurrentBufferPtr; }

Token Lexer::MakeToken(std::string_view Data, TokenType Type) const {
  return {Data, Type, CurrentLineNo,
          Data.length() == 0 ? unsigned(CurrentColumnNo)
                             : unsigned(CurrentColumnNo - Data.length())};
}

} // namespace frontEnd
} // namespace weak