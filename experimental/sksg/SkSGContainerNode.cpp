/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGContainerNode.h"

namespace sksg {

ContainerNode::ContainerNode() {}

ContainerNode::~ContainerNode() {
    for (const auto& child : fChildren) {
        child->removeParent(this);
    }
}

void ContainerNode::addChild(sk_sp<RenderNode> node) {
    // should we allow duplicates?
    for (const auto& child : fChildren) {
        if (child == node) {
            return;
        }
    }

    node->addParent(this);
    fChildren.push_back(std::move(node));
}

void ContainerNode::removeChild(const sk_sp<RenderNode>& node) {
    int origCount = fChildren.count();
    for (int i = 0; i < origCount; ++i) {
        if (fChildren[i] == node) {
            fChildren.removeShuffle(i);
            node->removeParent(this);
            break;
        }
    }
    SkASSERT(fChildren.count() == origCount - 1);
}

void ContainerNode::onRender(SkCanvas* canvas) const {
    for (const auto& child : fChildren) {
        child->render(canvas);
    }
}

} // namespace sksg
