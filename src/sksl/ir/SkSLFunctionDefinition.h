/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDEFINITION
#define SKSL_FUNCTIONDEFINITION

#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLProgramElement.h"

namespace SkSL {

struct ASTNode;

/**
 * A function definition (a declaration plus an associated block of code).
 */
struct FunctionDefinition : public ProgramElement {
    static constexpr Kind kProgramElementKind = Kind::kFunction;

    FunctionDefinition(int offset,
                       const FunctionDeclaration* declaration,
                       std::unique_ptr<Statement> body,
                       std::unordered_set<const FunctionDeclaration*> referencedIntrinsics = {})
        : INHERITED(offset, FunctionDefinitionData{declaration, std::move(referencedIntrinsics),
                                                   nullptr}) {
        fStatementChildren.push_back(std::move(body));
    }

    const FunctionDeclaration& declaration() const {
        return *this->functionDefinitionData().fDeclaration;
    }

    std::unique_ptr<Statement>& body() {
        return this->fStatementChildren[0];
    }

    const std::unique_ptr<Statement>& body() const {
        return this->fStatementChildren[0];
    }

    const std::unordered_set<const FunctionDeclaration*>& referencedIntrinsics() const {
        return this->functionDefinitionData().fReferencedIntrinsics;
    }

    const ASTNode* source() const {
        return this->functionDefinitionData().fSource;
    }

    void setSource(const ASTNode* source) {
        this->functionDefinitionData().fSource = source;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<FunctionDefinition>(fOffset, &this->declaration(),
                                                    this->body()->clone(),
                                                    this->referencedIntrinsics());
    }

    String description() const override {
        return this->declaration().description() + " " + this->body()->description();
    }

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
