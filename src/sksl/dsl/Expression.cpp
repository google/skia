/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLWriter.h"

#include "src/sksl/dsl/Var.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"

namespace SkSL {

namespace dsl {

static std::unique_ptr<SkSL::Expression> check(std::unique_ptr<SkSL::Expression> expr) {
    // '!expr' is ambiguous due to Expression::operator!, and clang-tidy complains if we just do
    // '!expr.get()', so we extract the pointer first
    SkSL::Expression* ptr = expr.get();
    if (!ptr) {
        DSLWriter& dsl = DSLWriter::Instance();
        if (dsl.compiler().errorCount()) {
            dsl.reportError(dsl.compiler().errorText(/*showCount=*/false).c_str());
        }
    }
    return expr;
}

Expression::Expression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(std::move(expression)) {}

Expression::Expression(float value)
    : fExpression(std::make_unique<SkSL::FloatLiteral>(DSLWriter::Instance().context(),
                                                       /*offset=*/-1,
                                                       value)) {}

Expression::Expression(int value)
    : fExpression(std::make_unique<SkSL::IntLiteral>(DSLWriter::Instance().context(),
                                                     /*offset=*/-1,
                                                     value)) {}

Expression::Expression(const Var& var)
    : fExpression(std::make_unique<SkSL::VariableReference>(
                                                        /*offset=*/-1,
                                                        var.var(),
                                                        SkSL::VariableReference::RefKind::kRead)) {}

Expression Expression::operator=(Expression&& right) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    return Expression(check(ir.convertBinaryExpression(/*offset=*/-1, this->release(),
                                                       SkSL::Token::Kind::TK_EQ, right.release())));
}

#define OP(op, token)                                                                              \
Expression operator op(Expression left, Expression right) {                                        \
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();                                   \
    return Expression(check(ir.convertBinaryExpression(/*offset=*/-1, left.release(),              \
                                                 SkSL::Token::Kind::token, right.release())));     \
}

#define RWOP(op, token)                                                                            \
OP(op, token)                                                                                      \
Expression operator op(Var& left, Expression right) {                                              \
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();                                   \
    return Expression(check(ir.convertBinaryExpression(/*offset=*/-1,                              \
                    std::make_unique<SkSL::VariableReference>(/*offset=*/-1,                       \
                                                    left.var(),                                    \
                                                    SkSL::VariableReference::RefKind::kReadWrite), \
                    SkSL::Token::Kind::token, right.release())));                                  \
}

#define PREFIXOP(op, token)                                                                        \
Expression operator op(Expression expr) {                                                          \
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();                                   \
    return Expression(check(ir.convertPrefixExpression(SkSL::Token::Kind::token,                   \
                                                       expr.release())));                          \
}

#define POSTFIXOP(op, token)                                                                       \
Expression operator op(Expression expr, int) {                                                     \
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();                                   \
    return Expression(check(ir.convertPostfixExpression(expr.release(),                            \
                                                        SkSL::Token::Kind::token)));               \
}

OP(+, TK_PLUS)
RWOP(+=, TK_PLUSEQ)
OP(-, TK_MINUS)
RWOP(-=, TK_MINUSEQ)
OP(*, TK_STAR)
RWOP(*=, TK_STAREQ)
OP(/, TK_SLASH)
RWOP(/=, TK_SLASHEQ)
OP(%, TK_PERCENT)
RWOP(%=, TK_PERCENTEQ)
OP(<<, TK_SHL)
RWOP(<<=, TK_SHLEQ)
OP(>>, TK_SHR)
RWOP(>>=, TK_SHREQ)
OP(&&, TK_LOGICALAND)
OP(||, TK_LOGICALOR)
OP(&, TK_BITWISEAND)
RWOP(&=, TK_BITWISEANDEQ)
OP(|, TK_BITWISEOR)
RWOP(|=, TK_BITWISEOREQ)
OP(^, TK_BITWISEXOR)
RWOP(^=, TK_BITWISEXOREQ)
OP(==, TK_EQEQ)
OP(!=, TK_NEQ)
OP(>, TK_GT)
OP(<, TK_LT)
OP(>=, TK_GTEQ)
OP(<=, TK_LTEQ)

PREFIXOP(!, TK_LOGICALNOT)
PREFIXOP(~, TK_BITWISENOT)
PREFIXOP(++, TK_PLUSPLUS)
POSTFIXOP(++, TK_PLUSPLUS)
PREFIXOP(--, TK_MINUSMINUS)
POSTFIXOP(--, TK_MINUSMINUS)

Expression operator,(Expression left, Expression right) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    return Expression(check(ir.convertBinaryExpression(/*offset=*/-1,
                                                       left.release(),
                                                       SkSL::Token::Kind::TK_COMMA,
                                                       right.release())));
}

std::unique_ptr<SkSL::Expression> Expression::coerceAndRelease(const SkSL::Type& type) {
    // tripping this assert means we had an error occur somewhere else in DSL construction that
    // wasn't caught where it should have been
    SkASSERTF(!DSLWriter::Instance().compiler().errorCount(), "Unexpected SkSL DSL error: %s",
              DSLWriter::Instance().compiler().errorText().c_str());
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    return check(ir.coerce(this->release(), type));
}

static SkSL::String swizzle_component(SwizzleComponent c) {
    switch (c) {
        case R:
            return "r";
        case G:
            return "g";
        case B:
            return "b";
        case A:
            return "a";
        case X:
            return "x";
        case Y:
            return "y";
        case Z:
            return "z";
        case W:
            return "w";
        case ZERO:
            return "0";
        case ONE:
            return "1";
        default:
            SkUNREACHABLE;
    }
}

Expression Swizzle(Expression base, SwizzleComponent a) {
    return Expression(check(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a))));
}

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b) {
    return Expression(check(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a) +
                                                                         swizzle_component(b))));
}

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c) {
    return Expression(check(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a) +
                                                                         swizzle_component(b) +
                                                                         swizzle_component(c))));
}

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c,
                   SwizzleComponent d) {
    return Expression(check(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a) +
                                                                         swizzle_component(b) +
                                                                         swizzle_component(c) +
                                                                         swizzle_component(d))));
}

} // namespace dsl

} // namespace SkSL
