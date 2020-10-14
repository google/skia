/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLPool.h"

#include "src/sksl/ir/SkSLIRNode.h"

namespace SkSL {

void* Pool::AllocIRNode() {
    return malloc(sizeof(IRNode));
}

void Pool::FreeIRNode(void* node) {
    free(node);
}

}  // namespace SkSL
