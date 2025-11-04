#include "Evalvisitor.h"
#include <algorithm>
#include <cctype>

// ============ BigInt Implementation ============

void BigInt::removeLeadingZeros() {
    while (value.length() > 1 && value[0] == '0') {
        value = value.substr(1);
    }
    if (value == "0") negative = false;
}

bool BigInt::absGreater(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) return a.length() > b.length();
    return a > b;
}

std::string BigInt::absAdd(const std::string& a, const std::string& b) {
    std::string result;
    int carry = 0;
    int i = a.length() - 1, j = b.length() - 1;
    
    while (i >= 0 || j >= 0 || carry) {
        int sum = carry;
        if (i >= 0) sum += a[i--] - '0';
        if (j >= 0) sum += b[j--] - '0';
        result = char('0' + sum % 10) + result;
        carry = sum / 10;
    }
    return result;
}

std::string BigInt::absSub(const std::string& a, const std::string& b) {
    std::string result;
    int borrow = 0;
    int i = a.length() - 1, j = b.length() - 1;
    
    while (i >= 0) {
        int diff = (a[i--] - '0') - borrow;
        if (j >= 0) diff -= (b[j--] - '0');
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        result = char('0' + diff) + result;
    }
    
    while (result.length() > 1 && result[0] == '0') {
        result = result.substr(1);
    }
    return result;
}

std::string BigInt::absMul(const std::string& a, const std::string& b) {
    if (a == "0" || b == "0") return "0";
    std::vector<int> result(a.length() + b.length(), 0);
    
    for (int i = a.length() - 1; i >= 0; i--) {
        for (int j = b.length() - 1; j >= 0; j--) {
            int mul = (a[i] - '0') * (b[j] - '0');
            int p1 = i + j, p2 = i + j + 1;
            int sum = mul + result[p2];
            result[p2] = sum % 10;
            result[p1] += sum / 10;
        }
    }
    
    std::string str;
    bool leadingZero = true;
    for (int num : result) {
        if (num != 0) leadingZero = false;
        if (!leadingZero) str += char('0' + num);
    }
    return str.empty() ? "0" : str;
}

std::pair<std::string, std::string> BigInt::absDiv(const std::string& a, const std::string& b) {
    if (b == "0") throw std::runtime_error("Division by zero");
    if (!absGreater(a, b) && a != b) return {"0", a};
    if (a == b) return {"1", "0"};
    
    std::string quotient, remainder;
    for (char digit : a) {
        remainder += digit;
        while (remainder.length() > 1 && remainder[0] == '0') {
            remainder = remainder.substr(1);
        }
        
        int count = 0;
        while (absGreater(remainder, b) || remainder == b) {
            remainder = absSub(remainder, b);
            count++;
        }
        quotient += char('0' + count);
    }
    
    while (quotient.length() > 1 && quotient[0] == '0') {
        quotient = quotient.substr(1);
    }
    if (remainder.empty()) remainder = "0";
    
    return {quotient, remainder};
}

BigInt::BigInt() : value("0"), negative(false) {}

BigInt::BigInt(const std::string& s) {
    if (s.empty() || s == "-") {
        value = "0";
        negative = false;
        return;
    }
    
    negative = (s[0] == '-');
    value = negative ? s.substr(1) : s;
    removeLeadingZeros();
}

BigInt::BigInt(long long n) {
    negative = n < 0;
    value = std::to_string(negative ? -n : n);
    removeLeadingZeros();
}

BigInt::BigInt(int n) : BigInt((long long)n) {}

std::string BigInt::toString() const {
    return (negative ? "-" : "") + value;
}

double BigInt::toDouble() const {
    double result = 0.0;
    for (char c : value) {
        result = result * 10 + (c - '0');
    }
    return negative ? -result : result;
}

bool BigInt::toBool() const {
    return value != "0";
}

BigInt BigInt::operator+(const BigInt& other) const {
    if (negative == other.negative) {
        BigInt result;
        result.value = absAdd(value, other.value);
        result.negative = negative;
        result.removeLeadingZeros();
        return result;
    } else {
        if (absGreater(value, other.value)) {
            BigInt result;
            result.value = absSub(value, other.value);
            result.negative = negative;
            result.removeLeadingZeros();
            return result;
        } else if (value == other.value) {
            return BigInt();
        } else {
            BigInt result;
            result.value = absSub(other.value, value);
            result.negative = other.negative;
            result.removeLeadingZeros();
            return result;
        }
    }
}

BigInt BigInt::operator-(const BigInt& other) const {
    return *this + (-other);
}

BigInt BigInt::operator*(const BigInt& other) const {
    BigInt result;
    result.value = absMul(value, other.value);
    result.negative = (negative != other.negative) && result.value != "0";
    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::operator/(const BigInt& other) const {
    auto [q, r] = absDiv(value, other.value);
    BigInt result;
    result.value = q;
    result.negative = (negative != other.negative) && result.value != "0";
    
    // Python floor division: if signs differ and there's a remainder, subtract 1 from quotient
    if (negative != other.negative && r != "0") {
        result = result - BigInt(1);
    }
    
    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::operator%(const BigInt& other) const {
    auto [q, r] = absDiv(value, other.value);
    BigInt result;
    result.value = r;
    
    // Python modulo: result has same sign as divisor (other)
    // a = b*q + r where r has same sign as b
    if (r != "0") {
        if (negative && !other.negative) {
            // negative % positive: result = b - r
            result = other - BigInt(r);
        } else if (!negative && other.negative) {
            // positive % negative: result = -(|b| - r) = r - |b|
            result = BigInt(r) + other;  // other is negative, so this subtracts
        } else if (negative && other.negative) {
            // negative % negative: result is negative
            result.value = r;
            result.negative = true;
        } else {
            // positive % positive
            result.value = r;
            result.negative = false;
        }
    } else {
        result.value = "0";
        result.negative = false;
    }
    
    result.removeLeadingZeros();
    return result;
}

BigInt BigInt::operator-() const {
    BigInt result = *this;
    if (value != "0") result.negative = !negative;
    return result;
}

bool BigInt::operator<(const BigInt& other) const {
    if (negative != other.negative) return negative;
    if (negative) return absGreater(value, other.value);
    return !absGreater(value, other.value) && value != other.value;
}

bool BigInt::operator>(const BigInt& other) const {
    return other < *this;
}

bool BigInt::operator<=(const BigInt& other) const {
    return !(*this > other);
}

bool BigInt::operator>=(const BigInt& other) const {
    return !(*this < other);
}

bool BigInt::operator==(const BigInt& other) const {
    return negative == other.negative && value == other.value;
}

bool BigInt::operator!=(const BigInt& other) const {
    return !(*this == other);
}

// ============ Value Implementation ============

Value::Value() : type(ValueType::NONE) {}

Value::Value(bool b) : type(ValueType::BOOL), boolVal(b) {}

Value::Value(const BigInt& i) : type(ValueType::INT), intVal(i) {}

Value::Value(double f) : type(ValueType::FLOAT), floatVal(f) {}

Value::Value(const std::string& s) : type(ValueType::STRING), stringVal(s) {}

std::string Value::toString() const {
    switch (type) {
        case ValueType::NONE:
            return "None";
        case ValueType::BOOL:
            return boolVal ? "True" : "False";
        case ValueType::INT:
            return intVal.toString();
        case ValueType::FLOAT: {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(6) << floatVal;
            return oss.str();
        }
        case ValueType::STRING:
            return stringVal;
    }
    return "";
}

bool Value::toBool() const {
    switch (type) {
        case ValueType::NONE:
            return false;
        case ValueType::BOOL:
            return boolVal;
        case ValueType::INT:
            return intVal.toBool();
        case ValueType::FLOAT:
            return floatVal != 0.0;
        case ValueType::STRING:
            return !stringVal.empty();
    }
    return false;
}

Value Value::toInt() const {
    switch (type) {
        case ValueType::BOOL:
            return Value(BigInt(boolVal ? 1 : 0));
        case ValueType::INT:
            return *this;
        case ValueType::FLOAT:
            return Value(BigInt((long long)floatVal));
        case ValueType::STRING: {
            try {
                // Remove leading/trailing spaces
                std::string s = stringVal;
                s.erase(0, s.find_first_not_of(" \t\n\r"));
                s.erase(s.find_last_not_of(" \t\n\r") + 1);
                
                if (s.find('.') != std::string::npos) {
                    return Value(BigInt((long long)std::stod(s)));
                }
                return Value(BigInt(s));
            } catch (...) {
                return Value(BigInt(0));
            }
        }
        default:
            return Value(BigInt(0));
    }
}

Value Value::toFloat() const {
    switch (type) {
        case ValueType::BOOL:
            return Value(boolVal ? 1.0 : 0.0);
        case ValueType::INT:
            return Value(intVal.toDouble());
        case ValueType::FLOAT:
            return *this;
        case ValueType::STRING:
            try {
                return Value(std::stod(stringVal));
            } catch (...) {
                return Value(0.0);
            }
        default:
            return Value(0.0);
    }
}

Value Value::toStr() const {
    if (type == ValueType::STRING) return *this;
    return Value(toString());
}

Value Value::operator+(const Value& other) const {
    if (type == ValueType::STRING || other.type == ValueType::STRING) {
        return Value(toString() + other.toString());
    }
    if (type == ValueType::FLOAT || other.type == ValueType::FLOAT) {
        return Value(toFloat().floatVal + other.toFloat().floatVal);
    }
    if (type == ValueType::INT && other.type == ValueType::INT) {
        return Value(intVal + other.intVal);
    }
    return Value();
}

Value Value::operator-(const Value& other) const {
    if (type == ValueType::FLOAT || other.type == ValueType::FLOAT) {
        return Value(toFloat().floatVal - other.toFloat().floatVal);
    }
    if (type == ValueType::INT && other.type == ValueType::INT) {
        return Value(intVal - other.intVal);
    }
    return Value();
}

Value Value::operator*(const Value& other) const {
    // String repetition
    if (type == ValueType::STRING && other.type == ValueType::INT) {
        std::string result;
        BigInt count = other.intVal;
        long long n = count.toDouble();
        if (n > 0) {
            for (long long i = 0; i < n; i++) {
                result += stringVal;
            }
        }
        return Value(result);
    }
    if (type == ValueType::INT && other.type == ValueType::STRING) {
        std::string result;
        BigInt count = intVal;
        long long n = count.toDouble();
        if (n > 0) {
            for (long long i = 0; i < n; i++) {
                result += other.stringVal;
            }
        }
        return Value(result);
    }
    
    if (type == ValueType::FLOAT || other.type == ValueType::FLOAT) {
        return Value(toFloat().floatVal * other.toFloat().floatVal);
    }
    if (type == ValueType::INT && other.type == ValueType::INT) {
        return Value(intVal * other.intVal);
    }
    return Value();
}

Value Value::operator/(const Value& other) const {
    return Value(toFloat().floatVal / other.toFloat().floatVal);
}

Value Value::operator%(const Value& other) const {
    if (type == ValueType::INT && other.type == ValueType::INT) {
        return Value(intVal % other.intVal);
    }
    return Value();
}

Value Value::floordiv(const Value& other) const {
    if (type == ValueType::INT && other.type == ValueType::INT) {
        return Value(intVal / other.intVal);
    }
    double result = std::floor(toFloat().floatVal / other.toFloat().floatVal);
    return Value(result);
}

Value Value::operator-() const {
    if (type == ValueType::INT) {
        return Value(-intVal);
    }
    if (type == ValueType::FLOAT) {
        return Value(-floatVal);
    }
    return Value();
}

bool Value::operator<(const Value& other) const {
    if (type == ValueType::FLOAT || other.type == ValueType::FLOAT) {
        return toFloat().floatVal < other.toFloat().floatVal;
    }
    if (type == ValueType::INT && other.type == ValueType::INT) {
        return intVal < other.intVal;
    }
    if (type == ValueType::STRING && other.type == ValueType::STRING) {
        return stringVal < other.stringVal;
    }
    return false;
}

bool Value::operator>(const Value& other) const {
    return other < *this;
}

bool Value::operator<=(const Value& other) const {
    return !(*this > other);
}

bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}

bool Value::operator==(const Value& other) const {
    if (type != other.type) {
        if ((type == ValueType::INT || type == ValueType::FLOAT) &&
            (other.type == ValueType::INT || other.type == ValueType::FLOAT)) {
            return toFloat().floatVal == other.toFloat().floatVal;
        }
        return false;
    }
    
    switch (type) {
        case ValueType::NONE:
            return true;
        case ValueType::BOOL:
            return boolVal == other.boolVal;
        case ValueType::INT:
            return intVal == other.intVal;
        case ValueType::FLOAT:
            return floatVal == other.floatVal;
        case ValueType::STRING:
            return stringVal == other.stringVal;
    }
    return false;
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

// ============ EvalVisitor Implementation ============

EvalVisitor::EvalVisitor() {
    scopes.push_back(std::map<std::string, Value>());
}

void EvalVisitor::pushScope() {
    scopes.push_back(std::map<std::string, Value>());
}

void EvalVisitor::popScope() {
    if (scopes.size() > 1) {
        scopes.pop_back();
    }
}

void EvalVisitor::setVariable(const std::string& name, const Value& val) {
    // Set in current scope
    scopes.back()[name] = val;
}

Value EvalVisitor::getVariable(const std::string& name) {
    // Search from innermost to outermost scope
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return (*it)[name];
        }
    }
    return Value(); // Return None if not found
}

bool EvalVisitor::hasVariable(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

std::string EvalVisitor::parseString(const std::string& s) {
    std::string result;
    bool escaped = false;
    
    // Remove quotes
    std::string content = s.substr(1, s.length() - 2);
    
    for (size_t i = 0; i < content.length(); i++) {
        if (escaped) {
            switch (content[i]) {
                case 'n': result += '\n'; break;
                case 't': result += '\t'; break;
                case 'r': result += '\r'; break;
                case '\\': result += '\\'; break;
                case '\'': result += '\''; break;
                case '\"': result += '\"'; break;
                default: result += content[i]; break;
            }
            escaped = false;
        } else if (content[i] == '\\') {
            escaped = true;
        } else {
            result += content[i];
        }
    }
    
    return result;
}

std::any EvalVisitor::visitFile_input(Python3Parser::File_inputContext *ctx) {
    for (auto stmt : ctx->stmt()) {
        visit(stmt);
    }
    return nullptr;
}

std::any EvalVisitor::visitFuncdef(Python3Parser::FuncdefContext *ctx) {
    std::string name = ctx->NAME()->toString();
    FunctionDef func;
    func.suite = ctx->suite();
    
    if (ctx->parameters()->typedargslist()) {
        auto params = ctx->parameters()->typedargslist();
        for (auto tfpdef : params->tfpdef()) {
            func.params.push_back(tfpdef->NAME()->toString());
        }
        
        // Handle default values
        auto tests = params->test();
        int numDefaults = tests.size();
        int numParams = func.params.size();
        for (int i = 0; i < numDefaults; i++) {
            Value defaultVal = std::any_cast<Value>(visit(tests[i]));
            func.defaults[func.params[numParams - numDefaults + i]] = defaultVal;
        }
    }
    
    functions[name] = func;
    return nullptr;
}

std::any EvalVisitor::visitStmt(Python3Parser::StmtContext *ctx) {
    if (ctx->simple_stmt()) {
        return visit(ctx->simple_stmt());
    }
    if (ctx->compound_stmt()) {
        return visit(ctx->compound_stmt());
    }
    return nullptr;
}

std::any EvalVisitor::visitSimple_stmt(Python3Parser::Simple_stmtContext *ctx) {
    return visit(ctx->small_stmt());
}

std::any EvalVisitor::visitSmall_stmt(Python3Parser::Small_stmtContext *ctx) {
    if (ctx->expr_stmt()) {
        return visit(ctx->expr_stmt());
    }
    if (ctx->flow_stmt()) {
        return visit(ctx->flow_stmt());
    }
    return nullptr;
}

std::any EvalVisitor::visitExpr_stmt(Python3Parser::Expr_stmtContext *ctx) {
    auto testlists = ctx->testlist();
    
    if (testlists.size() == 1) {
        // Just evaluation, no assignment
        return visit(testlists[0]);
    }
    
    if (ctx->augassign()) {
        // Augmented assignment
        auto lhs = testlists[0];
        auto rhs = testlists[1];
        
        // Get variable names from lhs
        auto lhsTests = lhs->test();
        if (lhsTests.size() == 1) {
            std::string varName;
            if (lhsTests[0]->or_test() && 
                lhsTests[0]->or_test()->and_test().size() == 1 &&
                lhsTests[0]->or_test()->and_test(0)->not_test().size() == 1) {
                auto notTest = lhsTests[0]->or_test()->and_test(0)->not_test(0);
                if (notTest->comparison() && 
                    notTest->comparison()->arith_expr().size() == 1) {
                    auto arith = notTest->comparison()->arith_expr(0);
                    if (arith->term().size() == 1 && arith->term(0)->factor().size() == 1) {
                        auto factor = arith->term(0)->factor(0);
                        if (factor->atom_expr() && factor->atom_expr()->atom() && 
                            factor->atom_expr()->atom()->NAME()) {
                            varName = factor->atom_expr()->atom()->NAME()->toString();
                        }
                    }
                }
            }
            
            if (!varName.empty()) {
                Value oldVal = getVariable(varName);
                Value rhsVal = std::any_cast<Value>(visit(rhs));
                
                std::string op = ctx->augassign()->getText();
                Value newVal;
                if (op == "+=") newVal = oldVal + rhsVal;
                else if (op == "-=") newVal = oldVal - rhsVal;
                else if (op == "*=") newVal = oldVal * rhsVal;
                else if (op == "/=") newVal = oldVal / rhsVal;
                else if (op == "//=") newVal = oldVal.floordiv(rhsVal);
                else if (op == "%=") newVal = oldVal % rhsVal;
                
                setVariable(varName, newVal);
            }
        }
    } else {
        // Regular assignment (possibly chained)
        Value rhsVal = std::any_cast<Value>(visit(testlists.back()));
        
        // Assign to all lhs expressions (right to left, excluding the last which is rhs)
        for (int i = testlists.size() - 2; i >= 0; i--) {
            auto lhs = testlists[i];
            auto lhsTests = lhs->test();
            
            if (lhsTests.size() == 1) {
                // Single assignment
                std::string varName;
                if (lhsTests[0]->or_test() && 
                    lhsTests[0]->or_test()->and_test().size() == 1 &&
                    lhsTests[0]->or_test()->and_test(0)->not_test().size() == 1) {
                    auto notTest = lhsTests[0]->or_test()->and_test(0)->not_test(0);
                    if (notTest->comparison() && 
                        notTest->comparison()->arith_expr().size() == 1) {
                        auto arith = notTest->comparison()->arith_expr(0);
                        if (arith->term().size() == 1 && arith->term(0)->factor().size() == 1) {
                            auto factor = arith->term(0)->factor(0);
                            if (factor->atom_expr() && factor->atom_expr()->atom() && 
                                factor->atom_expr()->atom()->NAME()) {
                                varName = factor->atom_expr()->atom()->NAME()->toString();
                            }
                        }
                    }
                }
                
                if (!varName.empty()) {
                    setVariable(varName, rhsVal);
                }
            }
        }
    }
    
    return nullptr;
}

std::any EvalVisitor::visitFlow_stmt(Python3Parser::Flow_stmtContext *ctx) {
    if (ctx->break_stmt()) return visit(ctx->break_stmt());
    if (ctx->continue_stmt()) return visit(ctx->continue_stmt());
    if (ctx->return_stmt()) return visit(ctx->return_stmt());
    return nullptr;
}

std::any EvalVisitor::visitBreak_stmt(Python3Parser::Break_stmtContext *ctx) {
    throw BreakException();
}

std::any EvalVisitor::visitContinue_stmt(Python3Parser::Continue_stmtContext *ctx) {
    throw ContinueException();
}

std::any EvalVisitor::visitReturn_stmt(Python3Parser::Return_stmtContext *ctx) {
    if (ctx->testlist()) {
        Value val = std::any_cast<Value>(visit(ctx->testlist()));
        throw ReturnException(val);
    }
    throw ReturnException(Value());
}

std::any EvalVisitor::visitCompound_stmt(Python3Parser::Compound_stmtContext *ctx) {
    if (ctx->if_stmt()) return visit(ctx->if_stmt());
    if (ctx->while_stmt()) return visit(ctx->while_stmt());
    if (ctx->funcdef()) return visit(ctx->funcdef());
    return nullptr;
}

std::any EvalVisitor::visitIf_stmt(Python3Parser::If_stmtContext *ctx) {
    auto tests = ctx->test();
    auto suites = ctx->suite();
    
    for (size_t i = 0; i < tests.size(); i++) {
        Value condition = std::any_cast<Value>(visit(tests[i]));
        if (condition.toBool()) {
            visit(suites[i]);
            return nullptr;
        }
    }
    
    // Else clause
    if (suites.size() > tests.size()) {
        visit(suites.back());
    }
    
    return nullptr;
}

std::any EvalVisitor::visitWhile_stmt(Python3Parser::While_stmtContext *ctx) {
    while (true) {
        Value condition = std::any_cast<Value>(visit(ctx->test()));
        if (!condition.toBool()) break;
        
        try {
            visit(ctx->suite());
        } catch (BreakException&) {
            break;
        } catch (ContinueException&) {
            continue;
        }
    }
    return nullptr;
}

std::any EvalVisitor::visitSuite(Python3Parser::SuiteContext *ctx) {
    if (ctx->simple_stmt()) {
        return visit(ctx->simple_stmt());
    }
    for (auto stmt : ctx->stmt()) {
        visit(stmt);
    }
    return nullptr;
}

std::any EvalVisitor::visitTest(Python3Parser::TestContext *ctx) {
    return visit(ctx->or_test());
}

std::any EvalVisitor::visitOr_test(Python3Parser::Or_testContext *ctx) {
    Value result = std::any_cast<Value>(visit(ctx->and_test(0)));
    for (size_t i = 1; i < ctx->and_test().size(); i++) {
        if (result.toBool()) {
            return result;
        }
        result = std::any_cast<Value>(visit(ctx->and_test(i)));
    }
    return result;
}

std::any EvalVisitor::visitAnd_test(Python3Parser::And_testContext *ctx) {
    Value result = std::any_cast<Value>(visit(ctx->not_test(0)));
    for (size_t i = 1; i < ctx->not_test().size(); i++) {
        if (!result.toBool()) {
            return result;
        }
        result = std::any_cast<Value>(visit(ctx->not_test(i)));
    }
    return result;
}

std::any EvalVisitor::visitNot_test(Python3Parser::Not_testContext *ctx) {
    if (ctx->not_test()) {
        Value val = std::any_cast<Value>(visit(ctx->not_test()));
        return Value(!val.toBool());
    }
    return visit(ctx->comparison());
}

std::any EvalVisitor::visitComparison(Python3Parser::ComparisonContext *ctx) {
    Value result = std::any_cast<Value>(visit(ctx->arith_expr(0)));
    
    if (ctx->comp_op().empty()) {
        return result;
    }
    
    for (size_t i = 0; i < ctx->comp_op().size(); i++) {
        Value right = std::any_cast<Value>(visit(ctx->arith_expr(i + 1)));
        std::string op = ctx->comp_op(i)->getText();
        
        bool cmpResult;
        if (op == "<") cmpResult = result < right;
        else if (op == ">") cmpResult = result > right;
        else if (op == "<=") cmpResult = result <= right;
        else if (op == ">=") cmpResult = result >= right;
        else if (op == "==") cmpResult = result == right;
        else if (op == "!=") cmpResult = result != right;
        else cmpResult = false;
        
        if (!cmpResult) {
            return Value(false);
        }
        result = right;
    }
    
    return Value(true);
}

std::any EvalVisitor::visitArith_expr(Python3Parser::Arith_exprContext *ctx) {
    Value result = std::any_cast<Value>(visit(ctx->term(0)));
    
    for (size_t i = 0; i < ctx->addorsub_op().size(); i++) {
        Value right = std::any_cast<Value>(visit(ctx->term(i + 1)));
        std::string op = ctx->addorsub_op(i)->getText();
        
        if (op == "+") result = result + right;
        else if (op == "-") result = result - right;
    }
    
    return result;
}

std::any EvalVisitor::visitTerm(Python3Parser::TermContext *ctx) {
    Value result = std::any_cast<Value>(visit(ctx->factor(0)));
    
    for (size_t i = 0; i < ctx->muldivmod_op().size(); i++) {
        Value right = std::any_cast<Value>(visit(ctx->factor(i + 1)));
        std::string op = ctx->muldivmod_op(i)->getText();
        
        if (op == "*") result = result * right;
        else if (op == "/") result = result / right;
        else if (op == "//") result = result.floordiv(right);
        else if (op == "%") result = result % right;
    }
    
    return result;
}

std::any EvalVisitor::visitFactor(Python3Parser::FactorContext *ctx) {
    if (ctx->factor()) {
        Value val = std::any_cast<Value>(visit(ctx->factor()));
        std::string op = ctx->children[0]->toString();
        if (op == "-") return -val;
        return val;
    }
    return visit(ctx->atom_expr());
}

std::any EvalVisitor::visitAtom_expr(Python3Parser::Atom_exprContext *ctx) {
    if (ctx->trailer()) {
        // Function call - get function name directly
        std::string funcName;
        if (ctx->atom()->NAME()) {
            funcName = ctx->atom()->NAME()->toString();
        } else {
            return Value();
        }
        
        // Built-in functions
        if (funcName == "print") {
            if (ctx->trailer()->arglist()) {
                auto args = ctx->trailer()->arglist()->argument();
                for (size_t i = 0; i < args.size(); i++) {
                    if (i > 0) std::cout << " ";
                    Value arg = std::any_cast<Value>(visit(args[i]));
                    std::cout << arg.toString();
                }
            }
            std::cout << std::endl;
            return Value();
        } else if (funcName == "int") {
            if (ctx->trailer()->arglist()) {
                auto args = ctx->trailer()->arglist()->argument();
                if (args.size() > 0) {
                    Value arg = std::any_cast<Value>(visit(args[0]));
                    return arg.toInt();
                }
            }
            return Value(BigInt(0));
        } else if (funcName == "float") {
            if (ctx->trailer()->arglist()) {
                auto args = ctx->trailer()->arglist()->argument();
                if (args.size() > 0) {
                    Value arg = std::any_cast<Value>(visit(args[0]));
                    return arg.toFloat();
                }
            }
            return Value(0.0);
        } else if (funcName == "str") {
            if (ctx->trailer()->arglist()) {
                auto args = ctx->trailer()->arglist()->argument();
                if (args.size() > 0) {
                    Value arg = std::any_cast<Value>(visit(args[0]));
                    return arg.toStr();
                }
            }
            return Value("");
        } else if (funcName == "bool") {
            if (ctx->trailer()->arglist()) {
                auto args = ctx->trailer()->arglist()->argument();
                if (args.size() > 0) {
                    Value arg = std::any_cast<Value>(visit(args[0]));
                    return Value(arg.toBool());
                }
            }
            return Value(false);
        } else if (functions.find(funcName) != functions.end()) {
            // User-defined function
            FunctionDef& func = functions[funcName];
            
            pushScope();
            
            // Set parameters
            std::map<std::string, Value> passedArgs;
            
            if (ctx->trailer()->arglist()) {
                auto args = ctx->trailer()->arglist()->argument();
                size_t posArgIdx = 0;
                
                for (auto arg : args) {
                    auto tests = arg->test();
                    if (tests.size() == 2) {
                        // Keyword argument
                        std::string paramName = tests[0]->getText();
                        Value val = std::any_cast<Value>(visit(tests[1]));
                        passedArgs[paramName] = val;
                    } else {
                        // Positional argument
                        if (posArgIdx < func.params.size()) {
                            Value val = std::any_cast<Value>(visit(tests[0]));
                            passedArgs[func.params[posArgIdx]] = val;
                            posArgIdx++;
                        }
                    }
                }
            }
            
            // Set all parameters with passed args or defaults
            for (const auto& param : func.params) {
                if (passedArgs.find(param) != passedArgs.end()) {
                    setVariable(param, passedArgs[param]);
                } else if (func.defaults.find(param) != func.defaults.end()) {
                    setVariable(param, func.defaults[param]);
                } else {
                    setVariable(param, Value());
                }
            }
            
            Value returnVal;
            try {
                visit(func.suite);
            } catch (ReturnException& e) {
                returnVal = e.value;
            }
            
            popScope();
            return returnVal;
        }
        
        return Value();
    }
    
    // No trailer, just evaluate atom
    return visit(ctx->atom());
}

std::any EvalVisitor::visitAtom(Python3Parser::AtomContext *ctx) {
    if (ctx->NAME()) {
        return getVariable(ctx->NAME()->toString());
    }
    if (ctx->NUMBER()) {
        std::string num = ctx->NUMBER()->toString();
        if (num.find('.') != std::string::npos) {
            return Value(std::stod(num));
        } else {
            return Value(BigInt(num));
        }
    }
    if (ctx->STRING().size() > 0) {
        std::string result;
        for (auto str : ctx->STRING()) {
            result += parseString(str->toString());
        }
        return Value(result);
    }
    if (ctx->getText() == "None") {
        return Value();
    }
    if (ctx->getText() == "True") {
        return Value(true);
    }
    if (ctx->getText() == "False") {
        return Value(false);
    }
    if (ctx->test()) {
        return visit(ctx->test());
    }
    if (ctx->format_string()) {
        return visit(ctx->format_string());
    }
    return Value();
}

std::any EvalVisitor::visitFormat_string(Python3Parser::Format_stringContext *ctx) {
    std::string result;
    
    for (size_t i = 0; i < ctx->children.size(); i++) {
        auto child = ctx->children[i];
        std::string text = child->toString();
        
        if (text == "f\"" || text == "f'" || text == "\"" || text == "'") {
            continue;
        }
        
        if (auto testlist = dynamic_cast<Python3Parser::TestlistContext*>(child)) {
            // Expression inside {}
            auto tests = testlist->test();
            for (size_t j = 0; j < tests.size(); j++) {
                if (j > 0) result += " ";
                Value val = std::any_cast<Value>(visit(tests[j]));
                result += val.toString();
            }
        } else {
            // Literal text
            if (text.length() >= 2 && text[0] == '{' && text[text.length()-1] == '}') {
                // Skip the braces, we handle testlist separately
                continue;
            }
            result += text;
        }
    }
    
    return Value(result);
}

std::any EvalVisitor::visitTestlist(Python3Parser::TestlistContext *ctx) {
    if (ctx->test().size() == 1) {
        return visit(ctx->test(0));
    }
    // For multiple values, return the first one for now
    // (tuple support would be more complex)
    return visit(ctx->test(0));
}

std::any EvalVisitor::visitArgument(Python3Parser::ArgumentContext *ctx) {
    return visit(ctx->test(0));
}

// Unused visitor methods
std::any EvalVisitor::visitParameters(Python3Parser::ParametersContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitTypedargslist(Python3Parser::TypedargslistContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitAugassign(Python3Parser::AugassignContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitComp_op(Python3Parser::Comp_opContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitAddorsub_op(Python3Parser::Addorsub_opContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitMuldivmod_op(Python3Parser::Muldivmod_opContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitTrailer(Python3Parser::TrailerContext *ctx) {
    return visitChildren(ctx);
}

std::any EvalVisitor::visitArglist(Python3Parser::ArglistContext *ctx) {
    return visitChildren(ctx);
}
