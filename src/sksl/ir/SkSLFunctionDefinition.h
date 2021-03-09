/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FUNCTIONDEFINITION
#define SKSL_FUNCTIONDEFINITION

#include "include/private/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"

namespace SkSL {

struct ASTNode;

/**
 * A function definition (a declaration plus an associated block of code).
 */
class FunctionDefinition final : public ProgramElement {
public:
    static constexpr Kind kProgramElementKind = Kind::kFunction;

    FunctionDefinition(int offset,
                       const FunctionDeclaration* declaration, bool builtin,
                       std::unique_ptr<Statement> body,
                       std::unordered_set<const FunctionDeclaration*> referencedIntrinsics = {})
        : INHERITED(offset, kProgramElementKind)
        , fDeclaration(declaration)
        , fBuiltin(builtin)
        , fBody(std::move(body))
        , fReferencedIntrinsics(std::move(referencedIntrinsics))
        , fSource(nullptr) {}

    const FunctionDeclaration& declaration() const {
        return *fDeclaration;
    }

    bool isBuiltin() const {
        return fBuiltin;
    }

    std::unique_ptr<Statement>& body() {
        return fBody;
    }

    const std::unique_ptr<Statement>& body() const {
        return fBody;
    }

    const std::unordered_set<const FunctionDeclaration*>& referencedIntrinsics() const {
        return fReferencedIntrinsics;
    }

    const ASTNode* source() const {
        return fSource;
    }

    void setSource(const ASTNode* source) {
        fSource = source;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::make_unique<FunctionDefinition>(fOffset, &this->declaration(),
                                                    /*builtin=*/false, this->body()->clone(),
                                                    this->referencedIntrinsics());
    }

    String description() const override {
        return this->declaration().description() + " " + this->body()->description();
    }

private:
    const FunctionDeclaration* fDeclaration;
    bool fBuiltin;
    std::unique_ptr<Statement> fBody;
    // We track intrinsic functions we reference so that we can ensure that all of them end up
    // copied into the final output.
    std::unordered_set<const FunctionDeclaration*> fReferencedIntrinsics;
    // This pointer may be null, and even when non-null is not guaranteed to remain valid for
    // the entire lifespan of this object. The parse tree's lifespan is normally controlled by
    // IRGenerator, so the IRGenerator being destroyed or being used to compile another file
    // will invalidate this pointer.
    const ASTNode* fSource;

    using INHERITED = ProgramElement;
};

}  // namespace SkSL

#endif
