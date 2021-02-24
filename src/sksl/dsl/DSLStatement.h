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
#include "src/sksl/dsl/DSLErrorHandling.h"

#include <memory>

class GrGLSLShaderBuilder;

namespace SkSL {

class Expression;
class Statement;

namespace dsl {

class DSLBlock;
class DSLExpression;
class DSLPossibleExpression;
class DSLPossibleStatement;
class DSLVar;

class DSLStatement {
public:
    DSLStatement() {}

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
};

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

} // namespace dsl

} // namespace SkSL

#endif
