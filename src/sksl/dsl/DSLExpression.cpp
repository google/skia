/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLExpression.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/DSLVar.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLIntLiteral.h"

namespace SkSL {

namespace dsl {

static std::unique_ptr<SkSL::Expression> check(std::unique_ptr<SkSL::Expression> expr) {
    if (expr == nullptr) {
        if (DSLWriter::Compiler().errorCount()) {
            DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        }
    }
    return expr;
}

DSLExpression::DSLExpression() {}

DSLExpression::DSLExpression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(check(std::move(expression))) {}

DSLExpression::DSLExpression(float value)
    : fExpression(std::make_unique<SkSL::FloatLiteral>(DSLWriter::Context(),
                                                       /*offset=*/-1,
                                                       value)) {}

DSLExpression::DSLExpression(int value)
    : fExpression(std::make_unique<SkSL::IntLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::DSLExpression(bool value)
    : fExpression(std::make_unique<SkSL::BoolLiteral>(DSLWriter::Context(),
                                                     /*offset=*/-1,
                                                     value)) {}

DSLExpression::DSLExpression(const DSLVar& var)
    : fExpression(std::make_unique<SkSL::VariableReference>(
                                                        /*offset=*/-1,
                                                        var.var(),
                                                        SkSL::VariableReference::RefKind::kRead)) {}

DSLExpression::~DSLExpression() {
    SkASSERTF(fExpression == nullptr,
              "Expression destroyed without being incorporated into output tree");
}

std::unique_ptr<SkSL::Expression> DSLExpression::release() {
    return std::move(fExpression);
}

DSLExpression DSLExpression::operator=(DSLExpression&& right) {
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();
    return DSLExpression(check(ir.convertBinaryExpression(this->release(), SkSL::Token::Kind::TK_EQ,
                                                          right.release())));
}

#define OP(op, token)                                                                              \
DSLExpression operator op(DSLExpression left, DSLExpression right) {                               \
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();                                              \
    return DSLExpression(check(ir.convertBinaryExpression(left.release(), SkSL::Token::Kind::token,\
                                                          right.release())));                      \
}

#define RWOP(op, token)                                                                            \
OP(op, token)                                                                                      \
DSLExpression operator op(DSLVar& left, DSLExpression right) {                                     \
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();                                              \
    return DSLExpression(check(ir.convertBinaryExpression(                                         \
                    std::make_unique<SkSL::VariableReference>(/*offset=*/-1,                       \
                                                    left.var(),                                    \
                                                    SkSL::VariableReference::RefKind::kReadWrite), \
                    SkSL::Token::Kind::token, right.release())));                                  \
}

#define PREFIXOP(op, token)                                                                        \
DSLExpression operator op(DSLExpression expr) {                                                    \
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();                                              \
    return DSLExpression(check(ir.convertPrefixExpression(SkSL::Token::Kind::token,                \
                                                          expr.release())));                       \
}

#define POSTFIXOP(op, token)                                                                       \
DSLExpression operator op(DSLExpression expr, int) {                                               \
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();                                              \
    return DSLExpression(check(ir.convertPostfixExpression(expr.release(),                         \
                                                           SkSL::Token::Kind::token)));            \
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

DSLExpression operator,(DSLExpression left, DSLExpression right) {
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();
    return DSLExpression(check(ir.convertBinaryExpression(left.release(),
                                                          SkSL::Token::Kind::TK_COMMA,
                                                          right.release())));
}

std::unique_ptr<SkSL::Expression> DSLExpression::coerceAndRelease(const SkSL::Type& type) {
    // tripping this assert means we had an error occur somewhere else in DSL construction that
    // wasn't caught where it should have been
    SkASSERTF(!DSLWriter::Compiler().errorCount(), "Unexpected SkSL DSL error: %s",
              DSLWriter::Compiler().errorText().c_str());
    return check(DSLWriter::IRGenerator().coerce(this->release(), type));
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

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a) {
    return DSLExpression(check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                       swizzle_component(a))));
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b) {
    return DSLExpression(check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                       swizzle_component(a) +
                                                                       swizzle_component(b))));
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                      SwizzleComponent c) {
    return DSLExpression(check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                       swizzle_component(a) +
                                                                       swizzle_component(b) +
                                                                       swizzle_component(c))));
}

DSLExpression Swizzle(DSLExpression base, SwizzleComponent a, SwizzleComponent b,
                      SwizzleComponent c, SwizzleComponent d) {
    return DSLExpression(check(DSLWriter::IRGenerator().convertSwizzle(base.release(),
                                                                       swizzle_component(a) +
                                                                       swizzle_component(b) +
                                                                       swizzle_component(c) +
                                                                       swizzle_component(d))));
}

} // namespace dsl

} // namespace SkSL
