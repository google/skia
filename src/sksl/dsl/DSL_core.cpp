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
    DSLWriter::SetErrorHandler(errorHandler);
}

// normally we would use std::make_unique to create the nodes below, but explicitly creating
// std::unique_ptr<SkSL::Statement> avoids issues with ambiguous constructor invocations

DSLStatement Declare(DSLVar& var, DSLExpression initialValue) {
    DSLWriter::SymbolTable()->add(std::move(var.fOwnedVar));
    return std::unique_ptr<SkSL::Statement>(new SkSL::VarDeclaration(
                                                 var.var(),
                                                 &var.var()->type(),
                                                 /*arraySize=*/0,
                                                 initialValue.coerceAndRelease(var.var()->type())));
}

DSLStatement Do(DSLStatement stmt, DSLExpression test) {
    const SkSL::Type& boolType = *DSLWriter::Context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::DoStatement(/*offset=*/-1,
                                                                  stmt.release(),
                                                                  test.coerceAndRelease(boolType)));
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt) {
    const SkSL::Type& boolType = *DSLWriter::Context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::ForStatement(/*offset=*/-1,
                                                                   initializer.release(),
                                                                   test.coerceAndRelease(boolType),
                                                                   next.release(),
                                                                   stmt.release(),
                                                                   nullptr));
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
    const SkSL::Type& boolType = *DSLWriter::Context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::IfStatement(/*offset=*/-1,
                                                                  /*isStatic=*/false,
                                                                  test.coerceAndRelease(boolType),
                                                                  ifTrue.release(),
                                                                  ifFalse.release()));
}

DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
    return DSLExpression(DSLWriter::IRGenerator().convertTernaryExpression(test.release(),
                                                                           ifTrue.release(),
                                                                           ifFalse.release()));
}

DSLStatement While(DSLExpression test, DSLStatement stmt) {
    const SkSL::Type& boolType = *DSLWriter::Context().fBool_Type;
    return std::unique_ptr<SkSL::Statement>(new SkSL::WhileStatement(
                                                                    /*offset=*/-1,
                                                                    test.coerceAndRelease(boolType),
                                                                    stmt.release()));
}

static void ignore(std::unique_ptr<SkSL::Expression>&) {}

template <typename... Args>
DSLExpression dsl_function(const char* name, Args... args) {
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();
    SkSL::ExpressionArray argArray;
    argArray.reserve_back(sizeof...(args));

    // in C++17, we could just do:
    // (argArray.push_back(args.release()), ...);
    int unused[] = {0, (ignore(argArray.push_back(args.release())), 0)...};
    static_cast<void>(unused);

    return ir.call(/*offset=*/-1, ir.convertIdentifier(-1, name), std::move(argArray));
}
DSLExpression ceil(DSLExpression x) {
    return dsl_function("ceil", std::move(x));
}

DSLExpression clamp(DSLExpression x, DSLExpression min, DSLExpression max) {
    return dsl_function("clamp", std::move(x), std::move(min), std::move(max));
}

DSLExpression dot(DSLExpression x, DSLExpression y) {
    return dsl_function("dot", std::move(x), std::move(y));
}

DSLExpression floor(DSLExpression x) {
    return dsl_function("floor", std::move(x));
}

DSLExpression saturate(DSLExpression x) {
    return dsl_function("saturate", std::move(x));
}

DSLExpression unpremul(DSLExpression x) {
    return dsl_function("unpremul", std::move(x));
}

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
DSLExpression sampleChild(int index, DSLExpression coords) {
    std::unique_ptr<SkSL::Expression> coordsExpr = coords.release();
    SkString code = DSLWriter::CurrentProcessor()->invokeChild(index, *DSLWriter::CurrentEmitArgs(),
                                                              coordsExpr ? coordsExpr->description()
                                                                         : "");
    return DSLExpression(std::make_unique<SkSL::CodeStringExpression>(code.c_str(),
                                                           DSLWriter::Context().fHalf4_Type.get()));
}

void Start(GrGLSLFragmentProcessor* currentProcessor,
           GrGLSLFragmentProcessor::EmitArgs* args) {
    DSLWriter::Push(currentProcessor, args);
    SkSL::IRGenerator& ir = DSLWriter::IRGenerator();
    ir.symbolTable()->add(std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                           DSLWriter::Modifiers(SkSL::Modifiers()),
                                                           args->fSampleCoord,
                                                           DSLWriter::Context().fFloat2_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
    ir.symbolTable()->add(std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                           DSLWriter::Modifiers(SkSL::Modifiers()),
                                                           args->fInputColor,
                                                           DSLWriter::Context().fHalf4_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
    ir.symbolTable()->add(std::make_unique<SkSL::Variable>(/*offset=*/-1,
                                                           DSLWriter::Modifiers(SkSL::Modifiers()),
                                                           args->fOutputColor,
                                                           DSLWriter::Context().fHalf4_Type.get(),
                                                           /*builtin=*/false,
                                                           SkSL::Variable::Storage::kLocal));
}

void End() {
    DSLWriter::Pop();
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

} // namespace dsl

} // namespace SkSL
