#pragma once
#ifndef PYTHON_INTERPRETER_EVALVISITOR_H
#define PYTHON_INTERPRETER_EVALVISITOR_H

#include "Python3ParserBaseVisitor.h"
#include <string>
#include <map>
#include <vector>
#include <any>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

// Big integer class for arbitrary precision arithmetic
class BigInt {
private:
    std::string value;
    bool negative;
    
    void removeLeadingZeros();
    static bool absGreater(const std::string& a, const std::string& b);
    static std::string absAdd(const std::string& a, const std::string& b);
    static std::string absSub(const std::string& a, const std::string& b);
    static std::string absMul(const std::string& a, const std::string& b);
    static std::pair<std::string, std::string> absDiv(const std::string& a, const std::string& b);
    
public:
    BigInt();
    BigInt(const std::string& s);
    BigInt(long long n);
    BigInt(int n);
    
    std::string toString() const;
    double toDouble() const;
    bool toBool() const;
    
    BigInt operator+(const BigInt& other) const;
    BigInt operator-(const BigInt& other) const;
    BigInt operator*(const BigInt& other) const;
    BigInt operator/(const BigInt& other) const;
    BigInt operator%(const BigInt& other) const;
    BigInt operator-() const;
    
    bool operator<(const BigInt& other) const;
    bool operator>(const BigInt& other) const;
    bool operator<=(const BigInt& other) const;
    bool operator>=(const BigInt& other) const;
    bool operator==(const BigInt& other) const;
    bool operator!=(const BigInt& other) const;
};

// Value type for interpreter
enum class ValueType { NONE, BOOL, INT, FLOAT, STRING };

class Value {
public:
    ValueType type;
    bool boolVal;
    BigInt intVal;
    double floatVal;
    std::string stringVal;
    
    Value();
    Value(bool b);
    Value(const BigInt& i);
    Value(double f);
    Value(const std::string& s);
    
    std::string toString() const;
    bool toBool() const;
    Value toInt() const;
    Value toFloat() const;
    Value toStr() const;
    
    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    Value floordiv(const Value& other) const;
    Value operator-() const;
    
    bool operator<(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>=(const Value& other) const;
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
};

// Exception classes
class BreakException {};
class ContinueException {};
class ReturnException {
public:
    Value value;
    ReturnException(const Value& v) : value(v) {}
};

// Function definition
struct FunctionDef {
    std::vector<std::string> params;
    std::map<std::string, Value> defaults;
    Python3Parser::SuiteContext* suite;
};

class EvalVisitor : public Python3ParserBaseVisitor {
private:
    std::vector<std::map<std::string, Value>> scopes;
    std::map<std::string, FunctionDef> functions;
    
    void pushScope();
    void popScope();
    void setVariable(const std::string& name, const Value& val);
    Value getVariable(const std::string& name);
    bool hasVariable(const std::string& name);
    
    std::string parseString(const std::string& s);
    Value evaluateFormatString(Python3Parser::Format_stringContext* ctx);
    
public:
    EvalVisitor();
    
    std::any visitFile_input(Python3Parser::File_inputContext *ctx) override;
    std::any visitFuncdef(Python3Parser::FuncdefContext *ctx) override;
    std::any visitParameters(Python3Parser::ParametersContext *ctx) override;
    std::any visitTypedargslist(Python3Parser::TypedargslistContext *ctx) override;
    std::any visitStmt(Python3Parser::StmtContext *ctx) override;
    std::any visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) override;
    std::any visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) override;
    std::any visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) override;
    std::any visitAugassign(Python3Parser::AugassignContext *ctx) override;
    std::any visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) override;
    std::any visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) override;
    std::any visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) override;
    std::any visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) override;
    std::any visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) override;
    std::any visitIf_stmt(Python3Parser::If_stmtContext *ctx) override;
    std::any visitWhile_stmt(Python3Parser::While_stmtContext *ctx) override;
    std::any visitSuite(Python3Parser::SuiteContext *ctx) override;
    std::any visitTest(Python3Parser::TestContext *ctx) override;
    std::any visitOr_test(Python3Parser::Or_testContext *ctx) override;
    std::any visitAnd_test(Python3Parser::And_testContext *ctx) override;
    std::any visitNot_test(Python3Parser::Not_testContext *ctx) override;
    std::any visitComparison(Python3Parser::ComparisonContext *ctx) override;
    std::any visitComp_op(Python3Parser::Comp_opContext *ctx) override;
    std::any visitArith_expr(Python3Parser::Arith_exprContext *ctx) override;
    std::any visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) override;
    std::any visitTerm(Python3Parser::TermContext *ctx) override;
    std::any visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) override;
    std::any visitFactor(Python3Parser::FactorContext *ctx) override;
    std::any visitAtom_expr(Python3Parser::Atom_exprContext *ctx) override;
    std::any visitTrailer(Python3Parser::TrailerContext *ctx) override;
    std::any visitAtom(Python3Parser::AtomContext *ctx) override;
    std::any visitFormat_string(Python3Parser::Format_stringContext *ctx) override;
    std::any visitTestlist(Python3Parser::TestlistContext *ctx) override;
    std::any visitArglist(Python3Parser::ArglistContext *ctx) override;
    std::any visitArgument(Python3Parser::ArgumentContext *ctx) override;
};

#endif//PYTHON_INTERPRETER_EVALVISITOR_H
