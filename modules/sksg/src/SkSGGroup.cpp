/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGGroup.h"

#include "include/core/SkCanvas.h"

#include <algorithm>

namespace sksg {

Group::Group(std::vector<sk_sp<RenderNode>> children)
    : fChildren(std::move(children)) {
    for (const auto& child : fChildren) {
        this->observeInval(child);
    }
}

Group::~Group() {
    for (const auto& child : fChildren) {
        this->unobserveInval(child);
    }
}

void Group::clear() {
    for (const auto& child : fChildren) {
        this->unobserveInval(child);
    }
    fChildren.clear();
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
    SkDEBUGCODE(const auto origSize = fChildren.size());
    fChildren.erase(std::remove(fChildren.begin(), fChildren.end(), node), fChildren.end());
    SkASSERT(fChildren.size() == origSize - 1);

    this->unobserveInval(node);
    this->invalidate();
}

void Group::onRender(SkCanvas* canvas, const RenderContext* ctx) const {
    const auto local_ctx = ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(),
                                                                         canvas->getTotalMatrix(),
                                                                         fRequiresIsolation);

    for (const auto& child : fChildren) {
        child->render(canvas, local_ctx);
    }
}

const RenderNode* Group::onNodeAt(const SkPoint& p) const {
    for (auto it = fChildren.crbegin(); it != fChildren.crend(); ++it) {
        if (const auto* node = (*it)->nodeAt(p)) {
            return node;
        }
    }

    return nullptr;
}

SkRect Group::onRevalidate(InvalidationController* ic, const SkMatrix& ctm) {
    SkASSERT(this->hasInval());

    SkRect bounds = SkRect::MakeEmpty();
    fRequiresIsolation = false;

    for (size_t i = 0; i < fChildren.size(); ++i) {
        const auto child_bounds = fChildren[i]->revalidate(ic, ctm);

        // If any of the child nodes overlap, group effects require layer isolation.
        if (!fRequiresIsolation && i > 0 && child_bounds.intersects(bounds)) {
#if 1
            // Testing conservatively against the union of prev bounds is cheap and good enough.
            fRequiresIsolation = true;
#else
            // Testing exhaustively doesn't seem to increase the layer elision rate in practice.
            for (size_t j = 0; j < i; ++ j) {
                if (child_bounds.intersects(fChildren[i]->bounds())) {
                    fRequiresIsolation = true;
                    break;
                }
            }
#endif
        }

        bounds.join(child_bounds);
    }

    return bounds;
}

} // namespace sksg
