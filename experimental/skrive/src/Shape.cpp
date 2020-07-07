/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkPaint.h"

namespace skrive {
namespace internal {

template <typename T>
size_t parse_node(StreamReader*, T*);

template <>
size_t parse_node<Shape>(StreamReader* sr, Shape* node) {
    const auto parent_id = parse_node<Drawable>(sr, node);

    node->setTransformAffectsStroke(sr->readBool("transformAffectsStroke"));

    return parent_id;
}

} // namespace internal

void Shape::onRevalidate() {
    this->INHERITED::onRevalidate();

    fFills.clear();
    fStrokes.clear();
    fGeometries.clear();

    for (const auto& child : this->children()) {
        if (const Paint* paint = *child) {
            SkASSERT(paint->style() == SkPaint::kFill_Style ||
                     paint->style() == SkPaint::kStroke_Style);

            auto& bucket = paint->style() == SkPaint::kFill_Style ? fFills : fStrokes;
            bucket.push_back(paint);
        } else if (const Geometry* geo = *child) {
            fGeometries.push_back(geo);
        }
    }

    SkDebugf("[Shape::onRevalidate] %zu geos %zu fill(s) %zu stroke(s)\n",
             fGeometries.size(), fFills.size(), fStrokes.size());
}

void Shape::onRender(SkCanvas* canvas) const {
    auto draw_paint = [this](SkCanvas* canvas, const Paint* paint) {
        SkPaint p;
        paint->apply(&p);

        for (const auto& geo : fGeometries) {
            geo->draw(canvas, p, paint->getFillRule());
        }
    };

    TransformableComponent::ScopedTransformContext stc(this, canvas);

    for (const auto* fill : fFills) {
        draw_paint(canvas, fill);
    }
    for (const auto* stroke : fStrokes) {
        draw_paint(canvas, stroke);
    }
}

} // namespace skrive
