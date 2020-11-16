/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSL.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLCodeStringExpression.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

namespace dsl {

Var sk_FragColor("sk_FragColor");
Var sk_FragCoord("sk_FragCoord");
thread_local Var sk_OutColor("<uninitialized>");

std::unique_ptr<SkSL::Statement> Block(SkSL::StatementArray stmts) {
    return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(stmts));
}

std::unique_ptr<SkSL::Statement> Do(Statement stmt, Expression test) {
    return std::make_unique<SkSL::DoStatement>(/*offset=*/-1, stmt.release(),
                                               test.coerceAndRelease(Type(kBool).skslType()));
}

std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue) {
    return std::make_unique<SkSL::IfStatement>(/*offset=*/-1, /*isStatic=*/false,
                                               test.coerceAndRelease(Type(kBool).skslType()),
                                               ifTrue.release(), nullptr);
}

std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue, Statement ifFalse) {
    return std::make_unique<SkSL::IfStatement>(/*offset=*/-1, /*isStatic=*/false,
                                               test.coerceAndRelease(Type(kBool).skslType()),
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
    return std::make_unique<SkSL::WhileStatement>(/*offset=*/-1,
                                                  test.coerceAndRelease(Type(kBool).skslType()),
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

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
Expression sampleChild(int index) {
    DSLWriter& dsl = DSLWriter::Instance();
    SkString code = dsl.currentProcessor()->invokeChild(index, *dsl.currentEmitArgs());
    return Expression(std::make_unique<SkSL::CodeStringExpression>(code.c_str(),
                                                                  dsl.context().fHalf4_Type.get()));
}

void Start(GrGLSLFragmentProcessor* currentProcessor,
           GrGLSLFragmentProcessor::EmitArgs* args) {
    DSLWriter& dsl = DSLWriter::Instance();
    SkASSERTF(!dsl.currentProcessor(), "Start() was called without End()");
    SkSL::IRGenerator& ir = dsl.irGenerator();
    ir.pushSymbolTable();
    ir.symbolTable()->add(std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                           dsl.modifiers(SkSL::Modifiers()),
                                                           args->fOutputColor,
                                                           dsl.context().fHalf4_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
    dsl.setCurrentProcessor(currentProcessor);
    dsl.setCurrentEmitArgs(args);
    dsl.setCurrentOutputName(args->fOutputColor);
}

void End() {
    DSLWriter& dsl = DSLWriter::Instance();
    SkASSERTF(dsl.currentProcessor(), "End() was called without Start()");
    dsl.irGenerator().popSymbolTable();
    dsl.setCurrentProcessor(nullptr);
    dsl.setCurrentEmitArgs(nullptr);
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

} // namespace dsl

} // namespace SkSL
