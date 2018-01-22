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
        this->unobserveInval(child);
    }
}

void Group::addChild(sk_sp<RenderNode> node) {
    // should we allow duplicates?
    for (const auto& child : fChildren) {
        if (child == node) {
            return;
        }
    }

    this->observeInval(node);
    fChildren.push_back(std::move(node));

    this->invalidate();
}

void Group::removeChild(const sk_sp<RenderNode>& node) {
    int origCount = fChildren.count();
    for (int i = 0; i < origCount; ++i) {
        if (fChildren[i] == node) {
            fChildren.removeShuffle(i);
            this->unobserveInval(node);
            break;
        }
    }
    SkASSERT(fChildren.count() == origCount - 1);

    this->invalidate();
}

void Group::onRender(SkCanvas* canvas) const {
    for (const auto& child : fChildren) {
        child->render(canvas);
    }
}

SkRect Group::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    SkRect bounds = SkRect::MakeEmpty();

    for (const auto& child : fChildren) {
        bounds.join(child->revalidate(ic, ctm));
    }

    return bounds;
}

} // namespace sksg
