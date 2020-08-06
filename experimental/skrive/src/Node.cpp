/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"

#include <tuple>

namespace skrive {
namespace internal {

template <typename T>
size_t parse_node(StreamReader*, T*);

template <>
size_t parse_node<Node>(StreamReader* sr, Node* node) {
    const auto parent_id = parse_node<TransformableComponent>(sr, node);

    node->setCollapsedVisibility(sr->readBool("isCollapsed"));

    if (sr->openArray("clips")) {
        const auto count = sr->readLength8();

        SkDebugf(".. %d clips\n", count);

        for (size_t i = 0; i < count; ++i) {
            if (sr->openObject("clip")) {
                /*const auto clip_id   =*/ sr->readUInt16("node");
                /*const auto intersect =*/ sr->readBool("intersect");

                // TODO: actually use clips
                sr->closeObject();
            }
        }

        sr->closeArray();
    }

    return parent_id;
}

}  // namespace internal

void Node::addChild(sk_sp<Component> child) {
    child->fParent = this;
    fChildren.push_back(std::move(child));
    this->invalidate();
}

void Node::onRevalidate() {
    SkASSERT(this->hasInval());

    for (const auto& child : fChildren) {
        if (child) {
            child->revalidate();
        }
    }
}

void Node::onRender(SkCanvas* canvas) const {
    SkASSERT(!this->hasInval());

    TransformableComponent::ScopedTransformContext stc(this, canvas);

    // TODO: draw order?
    for (const auto& child : this->children()) {
        child->render(canvas);
    }
}


}  // namespace skrive
