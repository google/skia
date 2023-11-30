/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/include/SkottieProperty.h"

#include "modules/skottie/src/SkottiePriv.h"
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
        && fDecorator == other.fDecorator
        && fLocale == other.fLocale
        && fFontFamily == other.fFontFamily;
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
ColorPropertyHandle::PropertyHandle(sk_sp<sksg::Color> node)
    : fNode(std::move(node)), fRevalidator(nullptr) {}

template <> SK_API
ColorPropertyHandle::PropertyHandle(const ColorPropertyHandle& other)
    : fNode(other.fNode), fRevalidator(other.fRevalidator) {}

template <> SK_API
ColorPropertyHandle::~PropertyHandle() {}

template <> SK_API
ColorPropertyValue ColorPropertyHandle::get() const {
    return fNode->getColor();
}

template <> SK_API
void ColorPropertyHandle::set(const ColorPropertyValue& c) {
    fNode->setColor(c);

    if (fRevalidator) {
        fRevalidator->revalidate();
    }
}

template <> SK_API
OpacityPropertyHandle::PropertyHandle(sk_sp<sksg::OpacityEffect> node)
    : fNode(std::move(node)), fRevalidator(nullptr) {}

template <> SK_API
OpacityPropertyHandle::PropertyHandle(const OpacityPropertyHandle& other)
    : fNode(other.fNode), fRevalidator(other.fRevalidator) {}

template <> SK_API
OpacityPropertyHandle::~PropertyHandle() {}

template <> SK_API
OpacityPropertyValue OpacityPropertyHandle::get() const {
    return fNode->getOpacity() * 100;
}

template <> SK_API
void OpacityPropertyHandle::set(const OpacityPropertyValue& o) {
    fNode->setOpacity(o / 100);

    if (fRevalidator) {
        fRevalidator->revalidate();
    }
}

template <> SK_API
TextPropertyHandle::PropertyHandle(sk_sp<internal::TextAdapter> node)
    : fNode(std::move(node)), fRevalidator(nullptr) {}

template <> SK_API
TextPropertyHandle::PropertyHandle(const TextPropertyHandle& other)
    : fNode(other.fNode), fRevalidator(other.fRevalidator) {}

template <> SK_API
TextPropertyHandle::~PropertyHandle() {}

template <> SK_API
TextPropertyValue TextPropertyHandle::get() const {
    return fNode->getText();
}

template<> SK_API
void TextPropertyHandle::set(const TextPropertyValue& t) {
    fNode->setText(t);

    if (fRevalidator) {
        fRevalidator->revalidate();
    }
}

template <> SK_API
TransformPropertyHandle::PropertyHandle(sk_sp<internal::TransformAdapter2D> node)
    : fNode(std::move(node)), fRevalidator(nullptr) {}

template <> SK_API
TransformPropertyHandle::PropertyHandle(const TransformPropertyHandle& other)
    : fNode(other.fNode), fRevalidator(other.fRevalidator) {}

template <> SK_API
TransformPropertyHandle::~PropertyHandle() {}

template <> SK_API
TransformPropertyValue TransformPropertyHandle::get() const {
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
void TransformPropertyHandle::set(const TransformPropertyValue& t) {
    fNode->setAnchorPoint(t.fAnchorPoint);
    fNode->setPosition(t.fPosition);
    fNode->setScale(t.fScale);
    fNode->setRotation(t.fRotation);
    fNode->setSkew(t.fSkew);
    fNode->setSkewAxis(t.fSkewAxis);

    if (fRevalidator) {
        fRevalidator->revalidate();
    }
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
