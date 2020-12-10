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

DSLExpression abs(DSLExpression x) {
    return dsl_function("abs", std::move(x));
}

DSLExpression all(DSLExpression x) {
    return dsl_function("all", std::move(x));
}

DSLExpression any(DSLExpression x) {
    return dsl_function("any", std::move(x));
}

DSLExpression ceil(DSLExpression x) {
    return dsl_function("ceil", std::move(x));
}

DSLExpression clamp(DSLExpression x, DSLExpression min, DSLExpression max) {
    return dsl_function("clamp", std::move(x), std::move(min), std::move(max));
}

DSLExpression cos(DSLExpression x) {
    return dsl_function("cos", std::move(x));
}

DSLExpression cross(DSLExpression x, DSLExpression y) {
    return dsl_function("cross", std::move(x), std::move(y));
}

DSLExpression degrees(DSLExpression x) {
    return dsl_function("degrees", std::move(x));
}

DSLExpression distance(DSLExpression x, DSLExpression y) {
    return dsl_function("distance", std::move(x), std::move(y));
}

DSLExpression dot(DSLExpression x, DSLExpression y) {
    return dsl_function("dot", std::move(x), std::move(y));
}

DSLExpression equal(DSLExpression x, DSLExpression y) {
    return dsl_function("equal", std::move(x), std::move(y));
}

DSLExpression exp(DSLExpression x) {
    return dsl_function("exp", std::move(x));
}

DSLExpression exp2(DSLExpression x) {
    return dsl_function("exp2", std::move(x));
}

DSLExpression faceforward(DSLExpression n, DSLExpression i, DSLExpression nref) {
    return dsl_function("faceforward", std::move(n), std::move(i), std::move(nref));
}

DSLExpression fract(DSLExpression x) {
    return dsl_function("fract", std::move(x));
}

DSLExpression floor(DSLExpression x) {
    return dsl_function("floor", std::move(x));
}

DSLExpression greaterThan(DSLExpression x, DSLExpression y) {
    return dsl_function("greaterThan", std::move(x), std::move(y));
}

DSLExpression greaterThanEqual(DSLExpression x, DSLExpression y) {
    return dsl_function("greaterThanEqual", std::move(x), std::move(y));
}

DSLExpression inverse(DSLExpression x) {
    return dsl_function("inverse", std::move(x));
}

DSLExpression inversesqrt(DSLExpression x) {
    return dsl_function("inversesqrt", std::move(x));
}

DSLExpression length(DSLExpression x) {
    return dsl_function("length", std::move(x));
}

DSLExpression lessThan(DSLExpression x, DSLExpression y) {
    return dsl_function("lessThan", std::move(x), std::move(y));
}

DSLExpression lessThanEqual(DSLExpression x, DSLExpression y) {
    return dsl_function("lessThanEqual", std::move(x), std::move(y));
}

DSLExpression log(DSLExpression x) {
    return dsl_function("log", std::move(x));
}

DSLExpression log2(DSLExpression x) {
    return dsl_function("log2", std::move(x));
}

DSLExpression max(DSLExpression x, DSLExpression y) {
    return dsl_function("max", std::move(x), std::move(y));
}

DSLExpression min(DSLExpression x, DSLExpression y) {
    return dsl_function("min", std::move(x), std::move(y));
}

DSLExpression mix(DSLExpression x, DSLExpression y, DSLExpression a) {
    return dsl_function("mix", std::move(x), std::move(y), std::move(a));
}

DSLExpression mod(DSLExpression x, DSLExpression y) {
    return dsl_function("mod", std::move(x), std::move(y));
}

DSLExpression normalize(DSLExpression x) {
    return dsl_function("normalize", std::move(x));
}

DSLExpression notEqual(DSLExpression x, DSLExpression y) {
    return dsl_function("notEqual", std::move(x), std::move(y));
}

DSLExpression pow(DSLExpression x, DSLExpression y) {
    return dsl_function("pow", std::move(x), std::move(y));
}

DSLExpression radians(DSLExpression x) {
    return dsl_function("radians", std::move(x));
}

DSLExpression reflect(DSLExpression i, DSLExpression n) {
    return dsl_function("reflect", std::move(i), std::move(n));
}

DSLExpression refract(DSLExpression i, DSLExpression n, DSLExpression eta) {
    return dsl_function("refract", std::move(i), std::move(n), std::move(eta));
}

DSLExpression saturate(DSLExpression x) {
    return dsl_function("saturate", std::move(x));
}

DSLExpression sign(DSLExpression x) {
    return dsl_function("sign", std::move(x));
}

DSLExpression sin(DSLExpression x) {
    return dsl_function("sin", std::move(x));
}

DSLExpression smoothstep(DSLExpression edge1, DSLExpression edge2, DSLExpression x) {
    return dsl_function("smoothstep", std::move(edge1), std::move(edge2), std::move(x));
}

DSLExpression sqrt(DSLExpression x) {
    return dsl_function("sqrt", std::move(x));
}

DSLExpression step(DSLExpression edge, DSLExpression x) {
    return dsl_function("step", std::move(edge), std::move(x));
}

DSLExpression tan(DSLExpression x) {
    return dsl_function("tan", std::move(x));
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
