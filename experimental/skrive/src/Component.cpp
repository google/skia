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
size_t parse_node<Component>(StreamReader* sr, Component* node) {
    node->setName(sr->readString("name"));

    const auto parent_id = sr->readId("parent");

    SkDebugf(".. %s -> %d\n", node->getName().c_str(), parent_id);

    return parent_id;
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

void Component::onRender(SkCanvas*) const {}

TransformableComponent::ScopedTransformContext::
ScopedTransformContext(const TransformableComponent* node, SkCanvas* canvas)
    : fCanvas(canvas)
    , fRestoreCount(canvas->getSaveCount()) {
    const auto lm = SkMatrix::Translate(node->getTranslation().x, node->getTranslation().y) *
                    SkMatrix::RotateDeg(node->getRotation()                               ) *
                    SkMatrix::Scale    (node->getScale().x      , node->getScale().y      );

    if (node->getOpacity() < 1) {
        SkPaint layer_paint;
        layer_paint.setAlphaf(node->getOpacity());
        canvas->saveLayer(nullptr, &layer_paint);
    } else if (!lm.isIdentity()) {
        canvas->save();
    }
    canvas->concat(lm);
}

TransformableComponent::ScopedTransformContext::~ScopedTransformContext() {
    fCanvas->restoreToCount(this->fRestoreCount);
}

} // namespace skrive
