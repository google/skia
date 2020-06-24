/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"

namespace skrive {
namespace internal {

template <typename T>
size_t parse_node(StreamReader*, T*);

template <>
size_t parse_node<Component>(StreamReader* sr, Component* node) {
    node->setName(sr->readString("name"));

    const auto parent_index = sr->readUInt16("parent");

    SkDebugf(".. %s -> %d\n", node->getName().c_str(), parent_index);

    return parent_index;
}

template <>
size_t parse_node<TransformableComponent>(StreamReader* sr, TransformableComponent* node) {
    const auto parent_index = parse_node<Component>(sr, node);

    node->setTranslation(sr->readV2("translation"));
    node->setRotation(sr->readFloat("rotation"));
    node->setScale(sr->readV2("scale"));
    node->setOpacity(sr->readFloat("opacity"));

    return parent_index;
}

} // namespace internal

void Component::invalidate() {
    auto* node = this;

    do {
        node->fDirty = true;
        node = node->fParent;
    } while (node && !node->hasInval());
}

void Component::revalidate() {
    if (this->hasInval()) {
        this->onRevalidate();
        fDirty = false;
    }
}

} // namespace skrive
