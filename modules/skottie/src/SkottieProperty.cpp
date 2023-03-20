/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SkottieProperty.h"

#include "modules/skottie/src/Transform.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/sksg/include/SkSGOpacityEffect.h"
#include "modules/sksg/include/SkSGPaint.h"

namespace skottie {

bool TextPropertyValue::operator==(const TextPropertyValue& other) const {
    return fTypeface == other.fTypeface
        && fText == other.fText
        && fTextSize == other.fTextSize
        && fStrokeWidth == other.fStrokeWidth
        && fLineHeight == other.fLineHeight
        && fLineShift == other.fLineShift
        && fAscent == other.fAscent
        && fMaxLines == other.fMaxLines
        && fHAlign == other.fHAlign
        && fVAlign == other.fVAlign
        && fResize == other.fResize
        && fLineBreak == other.fLineBreak
        && fDirection == other.fDirection
        && fCapitalization == other.fCapitalization
        && fBox == other.fBox
        && fFillColor == other.fFillColor
        && fStrokeColor == other.fStrokeColor
        && fPaintOrder == other.fPaintOrder
        && fStrokeJoin == other.fStrokeJoin
        && fHasFill == other.fHasFill
        && fHasStroke == other.fHasStroke
        && fDecorator == other.fDecorator;
}

bool TextPropertyValue::operator!=(const TextPropertyValue& other) const {
    return !(*this== other);
}

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

template <> SK_API
PropertyHandle<ColorPropertyValue, sksg::Color>::~PropertyHandle() {}

template <> SK_API
ColorPropertyValue PropertyHandle<ColorPropertyValue, sksg::Color>::get() const {
    return fNode->getColor();
}

template <> SK_API
void PropertyHandle<ColorPropertyValue, sksg::Color>::set(const ColorPropertyValue& c) {
    fNode->setColor(c);
}

template <> SK_API
PropertyHandle<OpacityPropertyValue, sksg::OpacityEffect>::~PropertyHandle() {}

template <> SK_API
OpacityPropertyValue PropertyHandle<OpacityPropertyValue, sksg::OpacityEffect>::get() const {
    return fNode->getOpacity() * 100;
}

template <> SK_API
void PropertyHandle<OpacityPropertyValue, sksg::OpacityEffect>::set(const OpacityPropertyValue& o) {
    fNode->setOpacity(o / 100);
}

template <> SK_API
PropertyHandle<TextPropertyValue, internal::TextAdapter>::~PropertyHandle() {}

template <> SK_API
TextPropertyValue PropertyHandle<TextPropertyValue, internal::TextAdapter>::get() const {
      return fNode->getText();
}

template<> SK_API
void PropertyHandle<TextPropertyValue, internal::TextAdapter>::set(const TextPropertyValue& t) {
      fNode->setText(t);
}

template <> SK_API
PropertyHandle<TransformPropertyValue, internal::TransformAdapter2D>::~PropertyHandle() {}

template <> SK_API
TransformPropertyValue PropertyHandle<TransformPropertyValue,
                                      internal::TransformAdapter2D>::get() const {
    return {
        fNode->getAnchorPoint(),
        fNode->getPosition(),
        fNode->getScale(),
        fNode->getRotation(),
        fNode->getSkew(),
        fNode->getSkewAxis()
    };
}

template <> SK_API
void PropertyHandle<TransformPropertyValue, internal::TransformAdapter2D>::set(
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

void PropertyObserver::onTextProperty(const char[],
                                      const LazyHandle<TextPropertyHandle>&) {}

void PropertyObserver::onTransformProperty(const char[],
                                           const LazyHandle<TransformPropertyHandle>&) {}

void PropertyObserver::onEnterNode(const char node_name[], NodeType) {}

void PropertyObserver::onLeavingNode(const char node_name[], NodeType) {}

}  // namespace skottie
