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
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

namespace dsl {

Var sk_FragColor("sk_FragColor");
Var sk_FragCoord("sk_FragCoord");
thread_local Var sk_SampleCoord("<uninitialized>");
thread_local Var sk_OutColor("<uninitialized>");
thread_local Var sk_InColor("<uninitialized>");

void SetErrorHandler(ErrorHandler* errorHandler) {
    DSLWriter::Instance().setErrorHandler(errorHandler);
}

std::unique_ptr<SkSL::Statement> Block(SkSL::StatementArray stmts) {
    return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(stmts));
}

std::unique_ptr<SkSL::Statement> Declare(Var& var, Expression initialValue) {
    return std::make_unique<SkSL::VarDeclaration>(var.var(), &var.var()->type(), ExpressionArray(),
                                                  initialValue.coerceAndRelease(var.var()->type()));
}

std::unique_ptr<SkSL::Statement> Do(Statement stmt, Expression test) {
    return std::make_unique<SkSL::DoStatement>(/*offset=*/-1, stmt.release(),
                                               test.coerceAndRelease(Type(kBool).skslType()));
}

std::unique_ptr<SkSL::Statement> For(Statement initializer, Expression test, Expression next,
                                     Statement stmt) {
    return std::make_unique<SkSL::ForStatement>(/*offset=*/-1, initializer.release(),
                                                test.release(), next.release(), stmt.release(),
                                                nullptr);
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

Expression clamp(Expression x, Expression min, Expression max) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    args.push_back(min.release());
    args.push_back(max.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "clamp"), std::move(args));
}

Expression dot(Expression x, Expression y) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    args.push_back(y.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "dot"), std::move(args));
}

Expression floor(Expression x) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "saturate"), std::move(args));
}

Expression saturate(Expression x) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "saturate"), std::move(args));
}

Expression unpremul(Expression x) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(x.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, "unpremul"), std::move(args));
}

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
Expression sampleChild(int index, Expression coords) {
    DSLWriter& dsl = DSLWriter::Instance();
    std::unique_ptr<SkSL::Expression> coordsExpr = coords.release();
    SkString code = dsl.currentProcessor()->invokeChild(index, *dsl.currentEmitArgs(),
                                                        coordsExpr ? coordsExpr->description()
                                                                   : "");
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
                                                           args->fSampleCoord,
                                                           dsl.context().fFloat2_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
    ir.symbolTable()->add(std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                           dsl.modifiers(SkSL::Modifiers()),
                                                           args->fInputColor,
                                                           dsl.context().fHalf4_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
    ir.symbolTable()->add(std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                           dsl.modifiers(SkSL::Modifiers()),
                                                           args->fOutputColor,
                                                           dsl.context().fHalf4_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
    dsl.setCurrentProcessor(currentProcessor);
    dsl.setCurrentEmitArgs(args);
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
