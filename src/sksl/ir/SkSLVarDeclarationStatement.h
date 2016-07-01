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
 * A variable declaration appearing as a statement within a function.
 */
struct VarDeclarationStatement : public Statement {
    VarDeclarationStatement(std::unique_ptr<VarDeclaration> decl)
    : INHERITED(decl->fPosition, kVarDeclaration_Kind) 
    , fDeclaration(std::move(decl)) {}

    std::string description() const override {
        return fDeclaration->description();
    }

    const std::shared_ptr<VarDeclaration> fDeclaration;

    typedef Statement INHERITED;
};

} // namespace

#endif
