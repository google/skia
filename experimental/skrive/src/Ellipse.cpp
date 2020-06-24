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
size_t parse_node<Ellipse>(StreamReader* sr, Ellipse* node) {
    const auto parent_index = parse_node<TransformableComponent>(sr, node);

    node->setWidth(sr->readFloat("width"));
    node->setHeight(sr->readFloat("height"));

    return parent_index;
}

} // namespace internal

void Ellipse::onRevalidate() {}

} // namespace skrive
