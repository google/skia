/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRNode.h"

#include "src/sksl/SkSLIRGenerator.h"

namespace SkSL {

IRNode& IRNode::ID::node() const {
    if (fNode) {
        return *fNode;
    }
    return (*fIRGenerator->fNodes)[fValue];
}

} // namespace
