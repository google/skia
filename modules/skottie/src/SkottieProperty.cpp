/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieProperty.h"

#include "SkottieAdapter.h"
#include "SkSGColor.h"
#include "SkSGOpacityEffect.h"

namespace skottie {

ColorPropertyProxy::ColorPropertyProxy(sk_sp<sksg::Color> color)
    : fColor(std::move(color)) {
    SkASSERT(fColor);
}

SkColor ColorPropertyProxy::getColor() const {
    return fColor->getColor();
}

void ColorPropertyProxy::setColor(SkColor color) {
    fColor->setColor(color);
}

OpacityPropertyProxy::OpacityPropertyProxy(sk_sp<sksg::OpacityEffect> opacity)
    : fOpacity(std::move(opacity)) {
    SkASSERT(opacity);
}

float OpacityPropertyProxy::getOpacity() const {
    return fOpacity->getOpacity();
}

void OpacityPropertyProxy::setOpacity(float opacity) {
    fOpacity->setOpacity(opacity);
}

TransformPropertyProxy::TransformPropertyProxy(sk_sp<TransformAdapter> transform)
    : fTransform(std::move(transform)) {
    SkASSERT(fTransform);
}

const SkPoint& TransformPropertyProxy::getPosition() const {
    return fTransform->getPosition();
}

void PropertyObserver::onColorProperty(const char[], std::unique_ptr<ColorPropertyProxy>) {}

void PropertyObserver::onOpacityProperty(const char[], std::unique_ptr<OpacityPropertyProxy>) {}

void PropertyObserver::onTransformProperty(const char[], std::unique_ptr<TransformPropertyProxy>) {}

} // namespace skottie
