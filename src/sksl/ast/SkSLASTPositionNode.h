/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTPOSITIONNODE
#define SKSL_ASTPOSITIONNODE

#include "SkSLASTNode.h"
#include "../SkSLPosition.h"

namespace SkSL {

/**
 * An AST node with an associated position in the source.
 */
struct ASTPositionNode : public ASTNode {
    ASTPositionNode(Position position)
    : fPosition(position) {}

    const Position fPosition;
};

} // namespace

#endif
