/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONSSTATEMENT
#define SKSL_VARDECLARATIONSSTATEMENT

#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"

namespace SkSL {

/**
 * One or more variable declarations appearing as a statement within a function.
 */
struct VarDeclarationsStatement : public Statement {
    VarDeclarationsStatement(std::unique_ptr<VarDeclarations> decl)
    : INHERITED(decl->fOffset, kVarDeclarations_Kind)
    , fDeclaration(std::move(decl)) {}

    bool isEmpty() const override {
        for (const auto& s : fDeclaration->fVars) {
            if (!s->isEmpty()) {
                return false;
            }
        }
        return true;
    }

    std::unique_ptr<Statement> clone() const override {
        std::unique_ptr<VarDeclarations> cloned((VarDeclarations*) fDeclaration->clone().release());
        return std::unique_ptr<Statement>(new VarDeclarationsStatement(std::move(cloned)));
    }

#ifdef SK_DEBUG
    String description() const override {
        return fDeclaration->description() + ";";
    }
#endif

    std::unique_ptr<VarDeclarations> fDeclaration;

    typedef Statement INHERITED;
};

} // namespace

#endif
