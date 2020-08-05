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

#include <set>

namespace SkSL {

struct ASTNode;

/**
 * A function definition (a declaration plus an associated block of code).
 */
struct FunctionDefinition : public ProgramElement {
    FunctionDefinition(int offset, const FunctionDeclaration& declaration,
                       std::unique_ptr<Statement> body,
                       std::set<const FunctionDeclaration*> referencedIntrinsics =
                                                             std::set<const FunctionDeclaration*>())
    : INHERITED(offset, kFunction_Kind)
    , fDeclaration(declaration)
    , fBody(std::move(body))
    , fReferencedIntrinsics(referencedIntrinsics) {}

    bool canBeInlined() const {
        static const int INLINE_THRESHOLD = 50; // chosen arbitrarily, feel free to adjust
        return fBody->nodeCount() < INLINE_THRESHOLD;
    }

    std::unique_ptr<ProgramElement> clone() const override {
        return std::unique_ptr<ProgramElement>(new FunctionDefinition(fOffset, fDeclaration,
                                                                      fBody->clone(),
                                                                      fReferencedIntrinsics));
    }

    String description() const override {
        return fDeclaration.description() + " " + fBody->description();
    }

    const FunctionDeclaration& fDeclaration;
    std::unique_ptr<Statement> fBody;
    // we track intrinsic functions we reference so that we can ensure that all of them end up
    // copied into the final output
    std::set<const FunctionDeclaration*> fReferencedIntrinsics;
    // This pointer may be null, and even when non-null is not guaranteed to remain valid for the
    // entire lifespan of this object. The parse tree's lifespan is normally controlled by
    // IRGenerator, so the IRGenerator being destroyed or being used to compile another file will
    // invalidate this pointer.
    const ASTNode* fSource = nullptr;

    typedef ProgramElement INHERITED;
};

}  // namespace SkSL

#endif
