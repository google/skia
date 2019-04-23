/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SkottieProperty.h"

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/sksg/include/SkSGOpacityEffect.h"
#include "modules/sksg/include/SkSGPaint.h"

namespace skottie {

bool TransformPropertyValue::operator==(const TransformPropertyValue& other) const {
    return this->fAnchorPoint == other.fAnchorPoint
        && this->fPosition    == other.fPosition
        && this->fScale       == other.fScale
        && this->fSkew        == other.fSkew
        && this->fSkewAxis    == other.fSkewAxis;
}

bool TransformPropertyValue::operator!=(const TransformPropertyValue& other) const {
    return !(*this == other);
}

template <>
PropertyHandle<ColorPropertyValue, sksg::Color>::~PropertyHandle() {}

template <>
ColorPropertyValue PropertyHandle<ColorPropertyValue, sksg::Color>::get() const {
    return fNode->getColor();
}

template <>
void PropertyHandle<ColorPropertyValue, sksg::Color>::set(const ColorPropertyValue& c) {
    fNode->setColor(c);
}

template <>
PropertyHandle<OpacityPropertyValue, sksg::OpacityEffect>::~PropertyHandle() {}

template <>
OpacityPropertyValue PropertyHandle<OpacityPropertyValue, sksg::OpacityEffect>::get() const {
    return fNode->getOpacity() * 100;
}

template <>
void PropertyHandle<OpacityPropertyValue, sksg::OpacityEffect>::set(const OpacityPropertyValue& o) {
    fNode->setOpacity(o / 100);
}

template <>
PropertyHandle<TransformPropertyValue, TransformAdapter2D>::~PropertyHandle() {}

template <>
TransformPropertyValue PropertyHandle<TransformPropertyValue, TransformAdapter2D>::get() const {
    return {
        fNode->getAnchorPoint(),
        fNode->getPosition(),
        fNode->getScale(),
        fNode->getRotation(),
        fNode->getSkew(),
        fNode->getSkewAxis()
    };
}

template <>
void PropertyHandle<TransformPropertyValue, TransformAdapter2D>::set(
        const TransformPropertyValue& t) {
    fNode->setAnchorPoint(t.fAnchorPoint);
    fNode->setPosition(t.fPosition);
    fNode->setScale(t.fScale);
    fNode->setRotation(t.fRotation);
    fNode->setSkew(t.fSkew);
    fNode->setSkewAxis(t.fSkewAxis);
}

void PropertyObserver::onColorProperty(const char[],
                                       const LazyHandle<ColorPropertyHandle>&) {}

void PropertyObserver::onOpacityProperty(const char[],
                                         const LazyHandle<OpacityPropertyHandle>&) {}

void PropertyObserver::onTransformProperty(const char[],
                                           const LazyHandle<TransformPropertyHandle>&) {}

} // namespace skottie
