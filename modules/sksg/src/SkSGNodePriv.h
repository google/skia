/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSGNodePriv_DEFINED
#define SkSGNodePriv_DEFINED

#include "modules/sksg/include/SkSGNode.h"

namespace sksg {

// Helper for accessing implementation-private Node methods.
class NodePriv final {
public:

    static bool HasInval(const sk_sp<Node>& node) { return node->hasInval(); }

private:
    NodePriv() = delete;
};

} // namespace sksg

#endif // SkSGNodePriv_DEFINED
