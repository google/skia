/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINEMARKER
#define SKSL_INLINEMARKER

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A no-op statement that indicates that a function was inlined here. This is necessary to detect
 * and prevent runaway infinite recursion. This node doesn't directly generate code.
 */
class InlineMarker final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kInlineMarker;

    InlineMarker(const FunctionDeclaration* function)
            : INHERITED(/*line=*/-1, kStatementKind)
            , fFunction(*function) {}

    static std::unique_ptr<Statement> Make(const FunctionDeclaration* function) {
        return std::make_unique<InlineMarker>(function);
    }

    const FunctionDeclaration& function() const {
        return fFunction;
    }

    bool isEmpty() const override {
        return true;
    }

    String description() const override {
        return String("/* inlined: ") + this->function().name() + String(" */");
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<InlineMarker>(&this->function());
    }

private:
    const FunctionDeclaration& fFunction;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
