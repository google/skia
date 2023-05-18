/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FUNCTION
#define SKSL_DSL_FUNCTION

#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/dsl/DSLExpression.h"
#include "src/sksl/dsl/DSLStatement.h"

#include <string_view>
#include <memory>

namespace SkSL {

class Context;
class ExpressionArray;
class FunctionDeclaration;
class Variable;

namespace dsl {

class DSLType;
struct DSLModifiers;

class DSLFunction {
public:
    DSLFunction(std::string_view name,
                const DSLModifiers& modifiers,
                const DSLType& returnType,
                skia_private::TArray<std::unique_ptr<SkSL::Variable>> parameters,
                Position pos = {});

    DSLFunction(SkSL::FunctionDeclaration* decl)
            : fDecl(decl) {}

    virtual ~DSLFunction() = default;

    void define(DSLStatement block, Position pos = {});

    void prototype();

    void addParametersToSymbolTable(const Context& context);

    /**
     * Invokes the function with the given arguments.
     */
    DSLExpression call(ExpressionArray args, Position pos = {});

private:
    void init(DSLModifiers modifiers,
              const DSLType& returnType,
              std::string_view name,
              skia_private::TArray<std::unique_ptr<SkSL::Variable>> params,
              Position pos);

    SkSL::FunctionDeclaration* fDecl = nullptr;
    SkSL::Position fPosition;
};

} // namespace dsl

} // namespace SkSL

#endif
