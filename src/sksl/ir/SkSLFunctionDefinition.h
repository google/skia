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

#include <unordered_set>

namespace SkSL {

struct ASTNode;

/**
 * A function definition (a declaration plus an associated block of code).
 */
struct FunctionDefinition : public IRNode {
    static constexpr Kind kIRNodeKind = kFunction_Kind;

    FunctionDefinition(int offset,
                       const FunctionDeclaration& declaration,
                       std::unique_ptr<Statement> body,
                       std::unordered_set<const FunctionDeclaration*> referencedIntrinsics = {})
        : INHERITED(offset, kIRNodeKind)
        , fDeclaration(declaration)
        , fBody(std::move(body))
        , fReferencedIntrinsics(std::move(referencedIntrinsics)) {}

    std::unique_ptr<IRNode> clone() const override {
        return std::make_unique<FunctionDefinition>(fOffset, fDeclaration,
                                                    fBody->cloneStatement(), fReferencedIntrinsics);
    }

    String description() const override {
        return fDeclaration.description() + " " + fBody->description();
    }

    const FunctionDeclaration& fDeclaration;
    std::unique_ptr<Statement> fBody;
    // We track intrinsic functions we reference so that we can ensure that all of them end up
    // copied into the final output.
    std::unordered_set<const FunctionDeclaration*> fReferencedIntrinsics;
    // This pointer may be null, and even when non-null is not guaranteed to remain valid for the
    // entire lifespan of this object. The parse tree's lifespan is normally controlled by
    // IRGenerator, so the IRGenerator being destroyed or being used to compile another file will
    // invalidate this pointer.
    const ASTNode* fSource = nullptr;

    typedef IRNode INHERITED;
};

}  // namespace SkSL

#endif
