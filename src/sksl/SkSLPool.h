/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_POOL
#define SKSL_POOL

namespace SkSL {

class IRNode;

class Pool {
public:
    static void* AllocIRNode();
    static void FreeIRNode(void* node);
};

}  // namespace SkSL

#endif
