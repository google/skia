/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRNODE
#define SKSL_IRNODE

#include "../SkSLPosition.h"

namespace SkSL {

/**
 * Represents a node in the intermediate representation (IR) tree. The IR is a fully-resolved
 * version of the program (all types determined, everything validated), ready for code generation.
 */
struct IRNode {
    IRNode(Position position)
    : fPosition(position) {}

    virtual ~IRNode() {}

    virtual String description() const = 0;

    const Position fPosition;
};

} // namespace

#endif
