/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"

namespace skrive::internal {
template <typename T>
size_t parse_node(StreamReader*, T*);

template <>
size_t parse_node<Drawable>(StreamReader* sr, Drawable* node) {
    const auto parent_id = parse_node<Node>(sr, node);

    node->setIsHidden(!sr->readBool("isVisible"));

    const auto bm = sr->readUInt8("blendMode");
    if (bm <= static_cast<uint8_t>(SkBlendMode::kLastMode)) {
        node->setBlendMode(static_cast<SkBlendMode>(bm));
    }

    node->setDrawOrder(sr->readUInt16("drawOrder"));

    return parent_id;
}

} // namespace skrive::internal
