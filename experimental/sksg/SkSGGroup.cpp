/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSGGroup.h"

namespace sksg {

Group::Group() {}

Group::~Group() {
    for (const auto& child : fChildren) {
        child->removeInvalReceiver(this);
    }
}

void Group::addChild(sk_sp<RenderNode> node) {
    // should we allow duplicates?
    for (const auto& child : fChildren) {
        if (child == node) {
            return;
        }
    }

    node->addInvalReceiver(this);
    fChildren.push_back(std::move(node));
}

void Group::removeChild(const sk_sp<RenderNode>& node) {
    int origCount = fChildren.count();
    for (int i = 0; i < origCount; ++i) {
        if (fChildren[i] == node) {
            fChildren.removeShuffle(i);
            node->removeInvalReceiver(this);
            break;
        }
    }
    SkASSERT(fChildren.count() == origCount - 1);
}

void Group::onRender(SkCanvas* canvas) const {
    for (const auto& child : fChildren) {
        child->render(canvas);
    }
}

void Group::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    for (const auto& child : fChildren) {
        child->revalidate(ic, ctm);
    }
}

} // namespace sksg
