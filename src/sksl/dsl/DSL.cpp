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
#ifdef SK_BUILD_FOR_UNIX
#include <sys/syscall.h>
#define gettid() ((int) syscall(SYS_gettid))
#endif

namespace SkSL {

namespace dsl {

Var sk_FragColor("sk_FragColor");
Var sk_FragCoord("sk_FragCoord");
Var sk_SampleCoord("sk_SampleCoord");
Var sk_OutColor("sk_OutColor");
Var sk_InColor("sk_InColor");

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

static Expression function1(const char* name, Expression a) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(a.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(args));
}

static Expression function2(const char* name, Expression a, Expression b) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(a.release());
    args.push_back(b.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(args));
}

static Expression function3(const char* name, Expression a, Expression b, Expression c) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(a.release());
    args.push_back(b.release());
    args.push_back(c.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(args));
}

Expression ceil(Expression x) {
    return function1("ceil", std::move(x));
}

Expression clamp(Expression x, Expression min, Expression max) {
    return function3("clamp", std::move(x), std::move(min), std::move(max));
}

Expression dot(Expression x, Expression y) {
    return function2("dot", std::move(x), std::move(y));
}

Expression floor(Expression x) {
    return function1("floor", std::move(x));
}

Expression saturate(Expression x) {
    return function1("saturate", std::move(x));
}

Expression unpremul(Expression x) {
    return function1("unpremul", std::move(x));
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
#ifdef SK_BUILD_FOR_UNIX
    printf("DSL Start, thread %d: %p: %p\n", gettid(), &dsl, dsl.currentProcessor());
#endif
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
#ifdef SK_BUILD_FOR_UNIX
    printf("/DSL Start, thread %d: %p: %p\n", gettid(), &dsl, dsl.currentProcessor());
#endif
}

void End() {
    DSLWriter& dsl = DSLWriter::Instance();
#ifdef SK_BUILD_FOR_UNIX
    printf("DSL End, thread %d: %p: %p\n", gettid(), &dsl, dsl.currentProcessor());
#endif
    SkASSERTF(dsl.currentProcessor(), "End() was called without Start()");
    dsl.irGenerator().popSymbolTable();
    dsl.setCurrentProcessor(nullptr);
    dsl.setCurrentEmitArgs(nullptr);
#ifdef SK_BUILD_FOR_UNIX
    printf("/DSL End, thread %d: %p: %p\n", gettid(), &dsl, dsl.currentProcessor());
#endif
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

} // namespace dsl

} // namespace SkSL
