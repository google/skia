/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_STATEMENT
#define SKSL_DSL_STATEMENT

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/DSLErrorHandling.h"

#include <memory>

class GrGLSLShaderBuilder;

namespace SkSL {

class Expression;
class Statement;

namespace dsl {

class DSLBlock;
class DSLExpression;
class DSLForLoopInitializer;
class DSLPossibleExpression;
class DSLPossibleStatement;
class DSLVar;

class DSLStatement {
public:
    DSLStatement();

    DSLStatement(DSLExpression expr);

    DSLStatement(DSLPossibleExpression expr, PositionInfo pos = PositionInfo());

    DSLStatement(DSLPossibleStatement stmt, PositionInfo pos = PositionInfo());

    DSLStatement(DSLBlock block);

    DSLStatement(DSLStatement&&) = default;

    ~DSLStatement();

    std::unique_ptr<SkSL::Statement> release() {
        return std::move(fStatement);
    }

private:
    DSLStatement(std::unique_ptr<SkSL::Statement> stmt);

    DSLStatement(std::unique_ptr<SkSL::Expression> expr);

    std::unique_ptr<SkSL::Statement> fStatement;

    friend class DSLBlock;
    friend class DSLCore;
    friend class DSLExpression;
    friend class DSLPossibleStatement;
    friend class DSLWriter;
    friend DSLForLoopInitializer operator,(DSLForLoopInitializer left, DSLForLoopInitializer right);
};

/**
 * Represents a Statement which may have failed and/or have pending errors to report. Converting a
 * PossibleStatement into a Statement requires PositionInfo so that any pending errors can be
 * reported at the correct position.
 *
 * PossibleStatement is used instead of Statement in situations where it is not possible to capture
 * the PositionInfo at the time of Statement construction.
 */
class DSLPossibleStatement {
public:
    DSLPossibleStatement(std::unique_ptr<SkSL::Statement> stmt);

    DSLPossibleStatement(DSLPossibleStatement&& other) = default;

    ~DSLPossibleStatement();

    std::unique_ptr<SkSL::Statement> release() {
        return std::move(fStatement);
    }

private:
    std::unique_ptr<SkSL::Statement> fStatement;

    friend class DSLStatement;
};

/**
 * Represents the results of applying the comma operator to statements, which is only legal in for
 * loop initializers. This is to prevent comma from appearing outside of a for loop initializer; the
 * opposite problem (things that are not legal in for loop initializers appearing in loop
 * initializers) is already error checked in ForStatement::Convert, so DSLForLoopInitializer does
 * not bother checking that its contents are in fact a legal for loop initializer.
 */
class DSLForLoopInitializer {
public:
    DSLForLoopInitializer(DSLStatement stmt);

    DSLForLoopInitializer(DSLPossibleStatement stmt, PositionInfo pos = PositionInfo());

    DSLForLoopInitializer(DSLExpression expr);

    DSLForLoopInitializer(DSLPossibleExpression expr, PositionInfo pos = PositionInfo());

private:
    DSLStatement fStatement;

    friend class DSLCore;
    friend DSLForLoopInitializer operator,(DSLForLoopInitializer left, DSLForLoopInitializer right);
};

DSLForLoopInitializer operator,(DSLForLoopInitializer left, DSLForLoopInitializer right);

} // namespace dsl

} // namespace SkSL

#endif
