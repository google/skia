/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTVARDECLARATIONSTATEMENT
#define SKSL_ASTVARDECLARATIONSTATEMENT

#include "SkSLASTStatement.h"
#include "SkSLASTVarDeclaration.h"

namespace SkSL {

/**
 * A variable declaration appearing as a statement within a function.
 */
struct ASTVarDeclarationStatement : public ASTStatement {
    ASTVarDeclarationStatement(std::unique_ptr<ASTVarDeclarations> decl)
    : INHERITED(decl->fPosition, kVarDeclaration_Kind)
    , fDeclarations(std::move(decl)) {}

    String description() const override {
        return fDeclarations->description() + ";";
    }

    std::unique_ptr<ASTVarDeclarations> fDeclarations;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
