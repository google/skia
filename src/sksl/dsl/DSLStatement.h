/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_STATEMENT
#define SKSL_DSL_STATEMENT

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/sksl/ir/SkSLIRNode.h"

#include <memory>

class GrGLSLShaderBuilder;

namespace SkSL {

class Statement;

namespace dsl {

class DSLBlock;
class DSLExpression;
class DSLVar;

class DSLStatement {
public:
    DSLStatement() {}

    DSLStatement(DSLExpression expr);

    DSLStatement(DSLBlock block);

    DSLStatement(DSLStatement&&) = default;

    ~DSLStatement() {
        SkASSERTF(!fStatement, "Statement destroyed without being incorporated into output tree");
    }

    std::unique_ptr<SkSL::Statement> release() {
        return std::move(fStatement);
    }

private:
    DSLStatement(std::unique_ptr<SkSL::Statement> stmt);

    DSLStatement(std::unique_ptr<SkSL::Expression> expr);

    std::unique_ptr<SkSL::Statement> fStatement;

    friend DSLStatement Declare(DSLVar& var, DSLExpression initialValue);
    friend DSLStatement Do(DSLStatement stmt, DSLExpression test);
    friend DSLStatement For(DSLStatement initializer, DSLExpression test, DSLExpression next,
                            DSLStatement stmt);
    friend DSLStatement If(DSLExpression test, DSLStatement ifTrue, DSLStatement ifFalse);
    friend DSLStatement While(DSLExpression test, DSLStatement stmt);

    friend class DSLBlock;
    friend class ::GrGLSLShaderBuilder;
};

} // namespace dsl

} // namespace SkSL

#endif
