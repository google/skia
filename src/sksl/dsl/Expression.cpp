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

Expression::Expression(std::unique_ptr<SkSL::Expression> expression)
    : fExpression(std::move(expression)) {
    if (!fExpression && DSLWriter::Instance().compiler().errorCount()) {
        printf("Error constructing SkSL code:\n%s\n",
               DSLWriter::Instance().compiler().errorText().c_str());
        // crashing here means you have an error in your SkSL DSL code. The error should have
        // been displayed by the above printf.
        SkUNREACHABLE;
    }
}

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
    return Expression(ir.convertBinaryExpression(/*offset=*/-1, this->release(),
                                                 SkSL::Token::Kind::TK_EQ, right.release()));
}

#define OP(op, token)                                                                              \
Expression operator op(Expression left, Expression right) {                                        \
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();                                   \
    return Expression(ir.convertBinaryExpression(/*offset=*/-1, left.release(),                    \
                                                 SkSL::Token::Kind::token, right.release()));      \
}

#define RWOP(op, token)                                                                            \
OP(op, token)                                                                                      \
Expression operator op(Var& left, Expression right) {                                              \
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();                                   \
    return Expression(ir.convertBinaryExpression(/*offset=*/-1,                                    \
                    std::make_unique<SkSL::VariableReference>(/*offset=*/-1,                       \
                                                    left.var(),                                    \
                                                    SkSL::VariableReference::RefKind::kReadWrite), \
                    SkSL::Token::Kind::token, right.release()));                                   \
}

OP(+, TK_PLUS)
RWOP(+=, TK_PLUSEQ)
OP(-, TK_MINUS)
RWOP(-=, TK_MINUSEQ)
OP(*, TK_STAR)
RWOP(*=, TK_STAREQ)
OP(/, TK_SLASH)
RWOP(/=, TK_SLASHEQ)
OP(==, TK_EQEQ)
OP(!=, TK_NEQ)
OP(>, TK_GT)
OP(<, TK_LT)
OP(>=, TK_GTEQ)
OP(<=, TK_LTEQ)

std::unique_ptr<SkSL::Expression> Expression::coerceAndRelease(const SkSL::Type& type) {
    // tripping this assert means we had an error occur somewhere else in DSL construction that
    // wasn't caught where it should have been
    SkASSERT(!DSLWriter::Instance().compiler().errorCount());
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    std::unique_ptr<SkSL::Expression> result = ir.coerce(this->release(), type);
    if (!result) {
        SkASSERT(DSLWriter::Instance().compiler().errorCount());
        printf("Error constructing SkSL code:\n%s\n",
               DSLWriter::Instance().compiler().errorText().c_str());
        // crashing here means you have a type mismatch in your SkSL DSL code. The error should
        // have been displayed by the above printf.
        SkUNREACHABLE;
    }
    SkASSERT(!DSLWriter::Instance().compiler().errorCount());
    return result;
}

} // namespace dsl

} // namespace SkSL
