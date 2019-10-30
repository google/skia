/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTFILE
#define SKSL_ASTFILE

#include "src/sksl/SkSLASTNode.h"

namespace SkSL {

struct ASTFile {
    ASTFile()
        : fRoot(ASTNode::ID::Invalid()) {}

    ASTNode& root() {
        return fNodes[fRoot.fValue];
    }

private:
    std::vector<ASTNode> fNodes;

    ASTNode::ID fRoot;

    friend class IRGenerator;
    friend class Parser;
};

} // namespace

#endif
