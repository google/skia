/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGContainerNode.h"

namespace sksg {

ContainerNode::ContainerNode() {}

void ContainerNode::addChild(sk_sp<Node> node) {
    // should we allow duplicates?
    for (const auto& child : fChildren) {
        if (child == node) {
            return;
        }
    }

    node->addRef(this);
    fChildren.push_back(std::move(node));
}

} // namespace sksg
