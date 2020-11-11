/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSL.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace skslcode {

Var sk_FragColor("sk_FragColor");
Var sk_FragCoord("sk_FragCoord");

std::unique_ptr<SkSL::Statement> Block(SkSL::StatementArray stmts) {
    return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(stmts));
}

std::unique_ptr<SkSL::Statement> Do(Statement stmt, Expression test) {
    return std::make_unique<SkSL::DoStatement>(/*offset=*/-1, stmt.release(),
                                               test.coerceAndRelease(Bool()));
}

std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue) {
    return std::make_unique<SkSL::IfStatement>(/*offset=*/-1, /*isStatic=*/false,
                                               test.coerceAndRelease(Bool()),
                                               ifTrue.release(), nullptr);
}

std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue, Statement ifFalse) {
    return std::make_unique<SkSL::IfStatement>(/*offset=*/-1, /*isStatic=*/false,
                                               test.coerceAndRelease(Bool()),
                                               ifTrue.release(), ifFalse.release());
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
    }
}

Expression Swizzle(Expression base, SwizzleComponent a) {
    return Expression(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a)));
}

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b) {
    return Expression(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a) +
                                                                         swizzle_component(b)));
}

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c) {
    return Expression(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a) +
                                                                         swizzle_component(b) +
                                                                         swizzle_component(c)));
}

Expression Swizzle(Expression base, SwizzleComponent a, SwizzleComponent b, SwizzleComponent c,
                   SwizzleComponent d) {
    return Expression(DSLWriter::Instance().irGenerator().convertSwizzle(base.release(),
                                                                         swizzle_component(a) +
                                                                         swizzle_component(b) +
                                                                         swizzle_component(c) +
                                                                         swizzle_component(d)));
}

Expression Ternary(Expression test, Expression ifTrue, Expression ifFalse) {
    return Expression(DSLWriter::Instance().irGenerator().convertTernaryExpression(
                                                                                -1,
                                                                                test.release(),
                                                                                ifTrue.release(),
                                                                                ifFalse.release()));
}

std::unique_ptr<SkSL::Statement> While(Expression test, Statement stmt) {
    return std::make_unique<SkSL::WhileStatement>(/*offset=*/-1, test.coerceAndRelease(Bool()),
                                                  stmt.release());
}

Expression dot(Expression x, Expression y) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    args.push_back(y.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "dot"), std::move(args));
}

Expression saturate(Expression x) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "saturate"), std::move(args));
}

void Start(const GrFragmentProcessor* currentProcessor,
           const GrGLSLFragmentProcessor::EmitArgs* args) {
    // Tripping this assert means Start() was called again before End()
    SkASSERT(!DSLWriter::Instance().currentProcessor());
    DSLWriter::Instance().setCurrentProcessor(currentProcessor);
    DSLWriter::Instance().setCurrentEmitArgs(args);
}

void End() {
    // Tripping this assert means End() was called without a matching Start()
    SkASSERT(DSLWriter::Instance().currentProcessor());
    DSLWriter::Instance().setCurrentProcessor(nullptr);
    DSLWriter::Instance().setCurrentEmitArgs(nullptr);
}

} // namespace skslcode
