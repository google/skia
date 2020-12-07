/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSL_core.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLCodeStringExpression.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

namespace dsl {

DSLVar sk_FragColor("sk_FragColor");
DSLVar sk_FragCoord("sk_FragCoord");
DSLVar sk_SampleCoord("sk_SampleCoord");
DSLVar sk_OutColor("sk_OutColor");
DSLVar sk_InColor("sk_InColor");

void SetErrorHandler(ErrorHandler* errorHandler) {
    DSLWriter::Instance().setErrorHandler(errorHandler);
}

// normally we would use std::make_unique to create the nodes below, but explicitly creating
// std::unique_ptr<SkSL::Statement> avoids issues with ambiguous constructor invocations

DSLStatement Declare(DSLVar& var) {
    DSLWriter::Instance().symbolTable()->add(std::move(var.fOwnedVar));
    return std::unique_ptr<SkSL::Statement>(new SkSL::VarDeclaration(var.var(), &var.var()->type(),
                                                                     0, nullptr));
}

DSLStatement Declare(DSLVar& var, DSLExpression initialValue) {
    DSLWriter::Instance().symbolTable()->add(std::move(var.fOwnedVar));
    return std::unique_ptr<SkSL::Statement>(new SkSL::VarDeclaration(
                                                 var.var(),
                                                 &var.var()->type(),
                                                 0,
                                                 initialValue.coerceAndRelease(var.var()->type())));
}

DSLStatement Do(DSLStatement stmt, DSLExpression test) {
    const SkSL::Type& boolType = *DSLWriter::Instance().context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::DoStatement(/*offset=*/-1,
                                                                  stmt.release(),
                                                                  test.coerceAndRelease(boolType)));
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt) {
    const SkSL::Type& boolType = *DSLWriter::Instance().context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::ForStatement(/*offset=*/-1,
                                                                   initializer.release(),
                                                                   test.coerceAndRelease(boolType),
                                                                   next.release(),
                                                                   stmt.release(),
                                                                   nullptr));
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue) {
    const SkSL::Type& boolType = *DSLWriter::Instance().context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::IfStatement(/*offset=*/-1,
                                                                  /*isStatic=*/false,
                                                                  test.coerceAndRelease(boolType),
                                                                  ifTrue.release(),
                                                                  nullptr));
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
    const SkSL::Type& boolType = *DSLWriter::Instance().context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::IfStatement(/*offset=*/-1,
                                                                  /*isStatic=*/false,
                                                                  test.coerceAndRelease(boolType),
                                                                  ifTrue.release(),
                                                                  ifFalse.release()));
}

DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
    const SkSL::Type& boolType = *DSLWriter::Instance().context().fBool_Type;
    return DSLExpression(DSLWriter::Instance().irGenerator().convertTernaryExpression(
                                                                    test.coerceAndRelease(boolType),
                                                                    ifTrue.release(),
                                                                    ifFalse.release()));
}

DSLStatement While(DSLExpression test, DSLStatement stmt) {
    const SkSL::Type& boolType = *DSLWriter::Instance().context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::WhileStatement(
                                                                    /*offset=*/-1,
                                                                    test.coerceAndRelease(boolType),
                                                                    stmt.release()));
}

DSLExpression dsl_function1(const char* name, DSLExpression a) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(a.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(args));
}

DSLExpression dsl_function2(const char* name, DSLExpression a, DSLExpression b) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(a.release());
    args.push_back(b.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(args));
}

DSLExpression dsl_function3(const char* name, DSLExpression a, DSLExpression b, DSLExpression c) {
    SkSL::IRGenerator& ir = DSLWriter::Instance().irGenerator();
    SkSL::ExpressionArray args;
    args.push_back(a.release());
    args.push_back(b.release());
    args.push_back(c.release());
    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(args));
}

DSLExpression ceil(DSLExpression x) {
    return dsl_function1("ceil", std::move(x));
}

DSLExpression clamp(DSLExpression x, DSLExpression min, DSLExpression max) {
    return dsl_function3("clamp", std::move(x), std::move(min), std::move(max));
}

DSLExpression dot(DSLExpression x, DSLExpression y) {
    return dsl_function2("dot", std::move(x), std::move(y));
}

DSLExpression floor(DSLExpression x) {
    return dsl_function1("floor", std::move(x));
}

DSLExpression saturate(DSLExpression x) {
    return dsl_function1("saturate", std::move(x));
}

DSLExpression unpremul(DSLExpression x) {
    return dsl_function1("unpremul", std::move(x));
}

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
DSLExpression sampleChild(int index, DSLExpression coords) {
    DSLWriter& dsl = DSLWriter::Instance();
    std::unique_ptr<SkSL::Expression> coordsExpr = coords.release();
    SkString code = dsl.currentProcessor()->invokeChild(index, *dsl.currentEmitArgs(),
                                                        coordsExpr ? coordsExpr->description()
                                                                   : "");
    return DSLExpression(std::make_unique<SkSL::CodeStringExpression>(code.c_str(),
                                                                  dsl.context().fHalf4_Type.get()));
}

void Start(GrGLSLFragmentProcessor* currentProcessor,
           GrGLSLFragmentProcessor::EmitArgs* args) {
    DSLWriter& dsl = DSLWriter::Instance();
    dsl.push(currentProcessor, args);
    SkSL::IRGenerator& ir = dsl.irGenerator();
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
}

void End() {
    DSLWriter::Instance().pop();
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

} // namespace dsl

} // namespace SkSL
