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

ColorPropertyHandle::ColorPropertyHandle(sk_sp<sksg::Color> color)
    : fColor(std::move(color)) {
    SkASSERT(fColor);
}

ColorPropertyHandle::~ColorPropertyHandle() = default;

SkColor ColorPropertyHandle::getColor() const {
    return fColor->getColor();
}

void ColorPropertyHandle::setColor(SkColor color) {
    fColor->setColor(color);
}

OpacityPropertyHandle::OpacityPropertyHandle(sk_sp<sksg::OpacityEffect> opacity)
    : fOpacity(std::move(opacity)) {
    SkASSERT(fOpacity);
}

OpacityPropertyHandle::~OpacityPropertyHandle() = default;

float OpacityPropertyHandle::getOpacity() const {
    return fOpacity->getOpacity() * 100;
}

void OpacityPropertyHandle::setOpacity(float opacity) {
    fOpacity->setOpacity(opacity / 100);
}

TransformPropertyHandle::TransformPropertyHandle(sk_sp<TransformAdapter> transform)
    : fTransform(std::move(transform)) {
    SkASSERT(fTransform);
}

TransformPropertyHandle::~TransformPropertyHandle() = default;

SkPoint TransformPropertyHandle::getAnchorPoint() const {
    return fTransform->getAnchorPoint();
}

void TransformPropertyHandle::setAnchorPoint(const SkPoint& ap) {
    fTransform->setAnchorPoint(ap);
}

SkPoint TransformPropertyHandle::getPosition() const {
    return fTransform->getPosition();
}

void TransformPropertyHandle::setPosition(const SkPoint& position) {
    fTransform->setPosition(position);
}

SkVector TransformPropertyHandle::getScale() const {
    return fTransform->getScale();
}

void TransformPropertyHandle::setScale(const SkVector& scale) {
    fTransform->setScale(scale);
}

SkScalar TransformPropertyHandle::getRotation() const {
    return fTransform->getRotation();
}

void TransformPropertyHandle::setRotation(SkScalar rotation) {
    fTransform->setRotation(rotation);
}

SkScalar TransformPropertyHandle::getSkew() const {
    return fTransform->getSkew();
}

void TransformPropertyHandle::setSkew(SkScalar skew) {
    fTransform->setSkew(skew);
}

SkScalar TransformPropertyHandle::getSkewAxis() const {
    return fTransform->getSkewAxis();
}

void TransformPropertyHandle::setSkewAxis(SkScalar sa) {
    fTransform->setSkewAxis(sa);
}

SkMatrix TransformPropertyHandle::getTotalMatrix() const {
    return fTransform->totalMatrix();
}

void PropertyObserver::onColorProperty(const char[],
                                       const LazyHandle<ColorPropertyHandle>&) {}

void PropertyObserver::onOpacityProperty(const char[],
                                         const LazyHandle<OpacityPropertyHandle>&) {}

void PropertyObserver::onTransformProperty(const char[],
                                           const LazyHandle<TransformPropertyHandle>&) {}

} // namespace skottie
