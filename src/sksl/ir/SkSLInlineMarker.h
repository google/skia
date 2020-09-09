/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINEMARKER
#define SKSL_INLINEMARKER

#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A no-op statement that indicates that a function was inlined here. This is necessary to detect
 * and prevent runaway infinite recursion. This node doesn't directly generate code.
 */
struct InlineMarker : public Statement {
    static constexpr Kind kStatementKind = Kind::kInlineMarker;

    InlineMarker(const FunctionDeclaration& funcDecl)
            : INHERITED(-1, kStatementKind), fFuncDecl(&funcDecl) {}

    bool isEmpty() const override {
        return true;
    }

    String description() const override {
        return String("/* inlined: ") + fFuncDecl->fName + String(" */");
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<InlineMarker>(*fFuncDecl);
    }

    const FunctionDeclaration* fFuncDecl;
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
