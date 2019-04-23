/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTPOSITIONNODE
#define SKSL_ASTPOSITIONNODE

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ast/SkSLASTNode.h"

namespace SkSL {

/**
 * An AST node with an associated position in the source.
 */
struct ASTPositionNode : public ASTNode {
    ASTPositionNode(int offset)
    : fOffset(offset) {}

    // character offset of this element within the program being compiled, for error reporting
    // purposes
    const int fOffset;
};

} // namespace

#endif
