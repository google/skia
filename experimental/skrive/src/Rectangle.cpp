/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkCanvas.h"

namespace skrive {
namespace internal {

template <typename T>
size_t parse_node(StreamReader*, T*);

template <>
size_t parse_node<Rectangle>(StreamReader* sr, Rectangle* node) {
    const auto parent_id = parse_node<Node>(sr, node);

    node->setWidth(sr->readFloat("width"));
    node->setHeight(sr->readFloat("height"));
    node->setRadius(sr->readFloat("cornerRadius"));

    return parent_id;
}

} // namespace internal

void Rectangle::onRevalidate() {
    SkASSERT(this->hasInval());
}

void Rectangle::onDraw(SkCanvas* canvas, const SkPaint& paint, SkPathFillType) const {
    SkASSERT(!this->hasInval());

    const auto rect = SkRect::MakeXYWH(-fWidth * 0.5f, -fHeight * 0.5f, fWidth, fHeight);

    canvas->drawRect(rect, paint);
}

} // namespace skrive
