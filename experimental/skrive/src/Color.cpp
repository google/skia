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

extern void parse_fill_stroke(StreamReader*, Paint*);

template <>
size_t parse_node<ColorPaint>(StreamReader* sr, ColorPaint* node) {
    const auto parent_id = parse_node<Paint>(sr, node);

    node->setColor(sr->readColor("color"));

    parse_fill_stroke(sr, node);

    return parent_id;
}

} // namespace internal

void ColorPaint::onRevalidate() {}

void ColorPaint::onApply(SkPaint* paint) const {
    this->INHERITED::onApply(paint);
    paint->setColor4f(fColor);
}

} // namespace skrive
