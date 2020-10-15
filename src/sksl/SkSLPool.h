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
    // The pool should be enabled when performing ephemeral work (converting a program). Don't burn
    // pool nodes on long-lived objects that all programs share, like rehydrated intrinsics or the
    // basic types.
    static void Enable();
    static void Disable();

    // Retrieves a node from the pool. If the pool is exhausted, this will allocate a node.
    static void* AllocIRNode();

    // Releases a node that was created by AllocIRNode. This will return it to the pool, or free it,
    // as appropriate. Make sure to free all nodes, since some of them may be real allocations.
    static void FreeIRNode(void* node_v);
};

}  // namespace SkSL

#endif
