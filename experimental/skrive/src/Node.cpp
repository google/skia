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
    node->setName(sr->readString("name"));
    const auto parent = sr->readUInt16("parent");

    node->setTranslation(sr->readV2("translation"));
    node->setRotation(sr->readFloat("rotation"));
    node->setScale(sr->readV2("scale"));
    node->setOpacity(sr->readFloat("opacity"));
    node->setCollapsedVisibility(sr->readBool("isCollapsed"));

    SkDebugf(".. '%s' -> %d\n", node->getName().c_str(), parent);

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

    return parent;
}

} // namespace skrive

SkRect Node::onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) {
    return this->INHERITED::onRevalidate(ic, ctm);
}

} // namespace internal
