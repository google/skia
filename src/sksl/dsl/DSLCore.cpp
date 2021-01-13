/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLCore.h"

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIfStatement.h"

namespace SkSL {

namespace dsl {

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
void Start(SkSL::Compiler* compiler) {
    DSLWriter::SetInstance(std::make_unique<DSLWriter>(compiler));
}

void End() {
    DSLWriter::SetInstance(nullptr);
}
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

void SetErrorHandler(ErrorHandler* errorHandler) {
    DSLWriter::SetErrorHandler(errorHandler);
}

// normally we would use std::make_unique to create the nodes below, but explicitly creating
// std::unique_ptr<SkSL::Statement> avoids issues with ambiguous constructor invocations

DSLStatement DSLCore::Declare(DSLVar& var, DSLExpression initialValue) {
    DSLWriter::SymbolTable()->add(std::move(var.fOwnedVar));
    return std::unique_ptr<SkSL::Statement>(new SkSL::VarDeclaration(
                                                 var.var(),
                                                 &var.var()->type(),
                                                 /*arraySize=*/0,
                                                 initialValue.coerceAndRelease(var.var()->type())));
}

DSLStatement DSLCore::Do(DSLStatement stmt, DSLExpression test) {
    const SkSL::Type& boolType = *DSLWriter::Context().fTypes.fBool;
    return std::unique_ptr<SkSL::Statement>(new SkSL::DoStatement(/*offset=*/-1,
                                                                  stmt.release(),
                                                                  test.coerceAndRelease(boolType)));
}

DSLStatement DSLCore::For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt) {
    const SkSL::Type& boolType = *DSLWriter::Context().fTypes.fBool;
    return std::unique_ptr<SkSL::Statement>(new SkSL::ForStatement(/*offset=*/-1,
                                                                   initializer.release(),
                                                                   test.coerceAndRelease(boolType),
                                                                   next.release(),
                                                                   stmt.release(),
                                                                   nullptr));
}

DSLStatement DSLCore::If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
    const SkSL::Type& boolType = *DSLWriter::Context().fTypes.fBool;
    return std::unique_ptr<SkSL::Statement>(new SkSL::IfStatement(/*offset=*/-1,
                                                                  /*isStatic=*/false,
                                                                  test.coerceAndRelease(boolType),
                                                                  ifTrue.release(),
                                                                  ifFalse.release()));
}

DSLExpression DSLCore::Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
    return DSLExpression(DSLWriter::IRGenerator().convertTernaryExpression(test.release(),
                                                                           ifTrue.release(),
                                                                           ifFalse.release()));
}

DSLStatement DSLCore::While(DSLExpression test, DSLStatement stmt) {
    return DSLWriter::IRGenerator().convertWhile(/*offset=*/-1, test.release(), stmt.release());
}

DSLStatement Declare(DSLVar& var, DSLExpression initialValue) {
    return DSLCore::Declare(var, std::move(initialValue));
}

DSLStatement Do(DSLStatement stmt, DSLExpression test) {
    return DSLCore::Do(std::move(stmt), std::move(test));
}

DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                 DSLStatement stmt) {
    return DSLCore::For(std::move(initializer), std::move(test), std::move(next), std::move(stmt));
}

DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse) {
    return DSLCore::If(std::move(test), std::move(ifTrue), std::move(ifFalse));
}

DSLExpression Ternary(DSLExpression test, DSLExpression ifTrue, DSLExpression ifFalse) {
    return DSLCore::Ternary(std::move(test), std::move(ifTrue), std::move(ifFalse));
}

DSLStatement While(DSLExpression test, DSLStatement stmt) {
    return DSLCore::While(std::move(test), std::move(stmt));
}

} // namespace dsl

} // namespace SkSL
