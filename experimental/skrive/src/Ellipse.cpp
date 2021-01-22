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
size_t parse_node<Ellipse>(StreamReader* sr, Ellipse* node) {
    const auto parent_id = parse_node<Node>(sr, node);

    node->setWidth(sr->readFloat("width"));
    node->setHeight(sr->readFloat("height"));

    return parent_id;
}

} // namespace internal

void Ellipse::onRevalidate() {
    SkASSERT(this->hasInval());
}

void Ellipse::onDraw(SkCanvas* canvas, const SkPaint& paint, SkPathFillType) const {
    SkASSERT(!this->hasInval());

    if (SkScalarNearlyEqual(fWidth, fHeight)) {
        canvas->drawCircle(0, 0, fWidth * 0.5f, paint);
    } else {
        canvas->drawOval(SkRect::MakeXYWH(-fWidth * 0.5f, -fHeight * 0.5f, fWidth, fHeight), paint);
    }
}

} // namespace skrive
