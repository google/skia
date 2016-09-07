/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_VARDECLARATIONSTATEMENT
#define SKSL_VARDECLARATIONSTATEMENT

#include "SkSLStatement.h"
#include "SkSLVarDeclaration.h"

namespace SkSL {

/**
 * One or more variable declarations appearing as a statement within a function.
 */
struct VarDeclarationsStatement : public Statement {
    VarDeclarationsStatement(std::unique_ptr<VarDeclarations> decl)
    : INHERITED(decl->fPosition, kVarDeclarations_Kind) 
    , fDeclaration(std::move(decl)) {}

    std::string description() const override {
        return fDeclaration->description();
    }

    const std::shared_ptr<VarDeclarations> fDeclaration;

    typedef Statement INHERITED;
};

} // namespace

#endif
