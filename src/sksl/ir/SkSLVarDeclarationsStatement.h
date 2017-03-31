/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VARDECLARATIONSSTATEMENT
#define SKSL_VARDECLARATIONSSTATEMENT

#include "SkSLStatement.h"
#include "SkSLVarDeclarations.h"

namespace SkSL {

/**
 * One or more variable declarations appearing as a statement within a function.
 */
struct VarDeclarationsStatement : public Statement {
    VarDeclarationsStatement(std::unique_ptr<VarDeclarations> decl)
    : INHERITED(decl->fPosition, kVarDeclarations_Kind)
    , fDeclaration(std::move(decl)) {}

    String description() const override {
        return fDeclaration->description();
    }

    std::shared_ptr<VarDeclarations> fDeclaration;

    typedef Statement INHERITED;
};

} // namespace

#endif
