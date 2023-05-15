/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"

#include <string_view>

namespace SkSL {

class ExpressionArray;
class FunctionDeclaration;

namespace dsl {

class DSLType;
struct DSLModifiers;
struct DSLParameter;

class DSLFunction {
public:
    DSLFunction(std::string_view name, const DSLModifiers& modifiers, const DSLType& returnType,
                SkSpan<DSLParameter*> parameters, Position pos = {});

    DSLFunction(SkSL::FunctionDeclaration* decl)
            : fDecl(decl) {}

    virtual ~DSLFunction() = default;

    void define(DSLStatement block, Position pos = {});

    void prototype();

    /**
     * Invokes the function with the given arguments.
     */
    DSLExpression call(ExpressionArray args, Position pos = {});

private:
    void init(DSLModifiers modifiers, const DSLType& returnType, std::string_view name,
              SkSpan<DSLParameter*> params, Position pos);

    SkSL::FunctionDeclaration* fDecl = nullptr;
    SkSL::Position fPosition;
};

} // namespace dsl

} // namespace SkSL

#endif
