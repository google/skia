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
    SkASSERT(fOpacity);
}

float OpacityPropertyProxy::getOpacity() const {
    return fOpacity->getOpacity() * 100;
}

void OpacityPropertyProxy::setOpacity(float opacity) {
    fOpacity->setOpacity(opacity / 100);
}

TransformPropertyProxy::TransformPropertyProxy(sk_sp<TransformAdapter> transform)
    : fTransform(std::move(transform)) {
    SkASSERT(fTransform);
}

const SkPoint& TransformPropertyProxy::getAnchorPoint() const {
    return fTransform->getAnchorPoint();
}

void TransformPropertyProxy::setAnchorPoint(const SkPoint& ap) {
    fTransform->setAnchorPoint(ap);
}

const SkPoint& TransformPropertyProxy::getPosition() const {
    return fTransform->getPosition();
}

void TransformPropertyProxy::setPosition(const SkPoint& position) {
    fTransform->setPosition(position);
}

const SkVector& TransformPropertyProxy::getScale() const {
    return fTransform->getScale();
}

void TransformPropertyProxy::setScale(const SkVector& scale) {
    fTransform->setScale(scale);
}

SkScalar TransformPropertyProxy::getRotation() const {
    return fTransform->getRotation();
}

void TransformPropertyProxy::setRotation(SkScalar rotation) {
    fTransform->setRotation(rotation);
}

SkScalar TransformPropertyProxy::getSkew() const {
    return fTransform->getSkew();
}

void TransformPropertyProxy::setSkew(SkScalar skew) {
    fTransform->setSkew(skew);
}

SkScalar TransformPropertyProxy::getSkewAxis() const {
    return fTransform->getSkewAxis();
}

void TransformPropertyProxy::setSkewAxis(SkScalar sa) {
    fTransform->setSkewAxis(sa);
}

SkMatrix TransformPropertyProxy::getTotalMatrix() const {
    return fTransform->totalMatrix();
}

void PropertyObserver::onColorProperty(const char[], std::unique_ptr<ColorPropertyProxy>) {}

void PropertyObserver::onOpacityProperty(const char[], std::unique_ptr<OpacityPropertyProxy>) {}

void PropertyObserver::onTransformProperty(const char[], std::unique_ptr<TransformPropertyProxy>) {}

} // namespace skottie
