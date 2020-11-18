/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL
#define SKSL_DSL

#include "src/sksl/SkSLContext.h"
#include "src/sksl/dsl/Expression.h"
#include "src/sksl/dsl/Function.h"
#include "src/sksl/dsl/Statement.h"
#include "src/sksl/dsl/Type.h"
#include "src/sksl/dsl/Var.h"
#include "src/sksl/dsl/priv/BlockCreator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"

namespace SkSL {

namespace dsl {

extern Var sk_FragColor;
extern Var sk_FragCoord;
// maybe avoid need for thread_local on iPhone by naming it some fixed token that gets string
// replaced?
thread_local extern Var sk_OutColor;

class ErrorHandler {
public:
    virtual ~ErrorHandler() {}

    virtual void handle(const char* msg) = 0;
};

void SetErrorHandler(ErrorHandler* errorHandler);

/**
 * Creates a block containing zero or more statements.
 */
template<class... Stmts>
std::unique_ptr<SkSL::Statement> Block(Stmts... stmts) {
    return BlockCreator<Stmts...>(std::move(stmts)...).block();
}

/**
 * do stmt; while (test);
 */
std::unique_ptr<SkSL::Statement> Do(Statement stmt, Expression test);

/**
 * if (test) ifTrue;
 */
std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue);

/**
 * if (test) ifTrue; else ifFalse;
 */
std::unique_ptr<SkSL::Statement> If(Expression test, Statement ifTrue, Statement ifFalse);

/**
 * test ? ifTrue : ifFalse
 */
Expression Ternary(Expression test, Expression ifTrue, Expression ifFalse);

/**
 * while (test) stmt;
 */
std::unique_ptr<SkSL::Statement> While(Expression test, Statement stmt);

/**
 * Computes the dot product of x and y.
 */
Expression dot(Expression x, Expression y);

#ifndef SKSL_STANDALONE
/**
 * Evaluates the index'th child of the current fragment processor at the current coordinates.
 */
Expression sampleChild(int index);
#endif // SKSL_STANDALONE

/**
 * Clamps the value of x to the range [0, 1].
 */
Expression saturate(Expression x);

#if SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)
/**
 * Begins DSL output within a fragment processor. This must be called prior to any other
 * interactions with the DSL API within a fragment processor, and must be paired with a matching
 * End() call.
 */
void Start(GrGLSLFragmentProcessor* currentProcessor, GrGLSLFragmentProcessor::EmitArgs* args);

/**
 * Must be called at the end of DSL output within a fragment processor.
 */
void End();
#endif // SK_SUPPORT_GPU && !defined(SKSL_STANDALONE)

} // namespace dsl

} // namespace SkSL

#endif
