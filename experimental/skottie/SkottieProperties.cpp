/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieProperties.h"

#include "SkColor.h"
#include "SkJSONCPP.h"
#include "SkPath.h"
#include "SkSGColor.h"
#include "SkSGGradient.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"

#include <cmath>

namespace  skottie {

namespace {

SkColor VecToColor(const float* v, size_t size) {
    // best effort to turn this into a color
    const auto r = size > 0 ? v[0] : 0,
               g = size > 1 ? v[1] : 0,
               b = size > 2 ? v[2] : 0,
               a = size > 3 ? v[3] : 1;

    return SkColorSetARGB(SkTPin<SkScalar>(a, 0, 1) * 255,
                          SkTPin<SkScalar>(r, 0, 1) * 255,
                          SkTPin<SkScalar>(g, 0, 1) * 255,
                          SkTPin<SkScalar>(b, 0, 1) * 255);
}

} // namespace

template <>
size_t ValueTraits<ScalarValue>::Cardinality(const ScalarValue&) {
    return 1;
}

template <>
template <>
SkScalar ValueTraits<ScalarValue>::As<SkScalar>(const ScalarValue& v) {
    return v;
}

template <>
size_t ValueTraits<VectorValue>::Cardinality(const VectorValue& vec) {
    return vec.size();
}

template <>
template <>
SkColor ValueTraits<VectorValue>::As<SkColor>(const VectorValue& vec) {
    return VecToColor(vec.data(), vec.size());
}

template <>
template <>
SkPoint ValueTraits<VectorValue>::As<SkPoint>(const VectorValue& vec) {
    // best effort to turn this into a point
    const auto x = vec.size() > 0 ? vec[0] : 0,
               y = vec.size() > 1 ? vec[1] : 0;
    return SkPoint::Make(x, y);
}

template <>
template <>
SkSize ValueTraits<VectorValue>::As<SkSize>(const VectorValue& vec) {
    const auto pt = ValueTraits::As<SkPoint>(vec);
    return SkSize::Make(pt.x(), pt.y());
}

template <>
size_t ValueTraits<ShapeValue>::Cardinality(const ShapeValue& path) {
    return SkTo<size_t>(path.countVerbs());
}

template <>
template <>
SkPath ValueTraits<ShapeValue>::As<SkPath>(const ShapeValue& path) {
    return path;
}

CompositeRRect::CompositeRRect(sk_sp<sksg::RRect> wrapped_node)
    : fRRectNode(std::move(wrapped_node)) {}

void CompositeRRect::apply() {
    // BM "position" == "center position"
    auto rr = SkRRect::MakeRectXY(SkRect::MakeXYWH(fPosition.x() - fSize.width() / 2,
                                                   fPosition.y() - fSize.height() / 2,
                                                   fSize.width(), fSize.height()),
                                  fRadius.width(),
                                  fRadius.height());
   fRRectNode->setRRect(rr);
}

CompositeTransform::CompositeTransform(sk_sp<sksg::Matrix> matrix)
    : fMatrixNode(std::move(matrix)) {}

void CompositeTransform::apply() {
    SkMatrix t = SkMatrix::MakeTrans(-fAnchorPoint.x(), -fAnchorPoint.y());

    t.postScale(fScale.x() / 100, fScale.y() / 100); // 100% based
    t.postRotate(fRotation);
    t.postTranslate(fPosition.x(), fPosition.y());
    // TODO: skew

    fMatrixNode->setMatrix(t);
}

CompositePolyStar::CompositePolyStar(sk_sp<sksg::Path> wrapped_node, Type t)
    : fPathNode(std::move(wrapped_node))
    , fType(t) {}

void CompositePolyStar::apply() {
    const auto count = SkScalarTruncToInt(fPointCount);
    const auto arc   = SK_ScalarPI * 2 / count;

    const auto pt_on_circle = [](const SkPoint& c, SkScalar r, SkScalar a) {
        return SkPoint::Make(c.x() + r * std::cos(a),
                             c.y() + r * std::sin(a));
    };

    // TODO: inner/outer "roundness"?

    SkPath poly;

    auto angle = SkDegreesToRadians(fRotation);
    poly.moveTo(pt_on_circle(fPosition, fOuterRadius, angle));

    for (int i = 0; i < count; ++i) {
        if (fType == Type::kStar) {
            poly.lineTo(pt_on_circle(fPosition, fInnerRadius, angle + arc * 0.5f));
        }
        angle += arc;
        poly.lineTo(pt_on_circle(fPosition, fOuterRadius, angle));
    }

    poly.close();
    fPathNode->setPath(poly);
}

CompositeGradient::CompositeGradient(sk_sp<sksg::Gradient> grad, size_t stopCount)
    : fGradient(std::move(grad))
    , fStopCount(stopCount) {}

void CompositeGradient::apply() {
    this->onApply();

    // |fColorStops| holds |fStopCount| x [ pos, r, g, g ] + ? x [ pos, alpha ]

    if (fColorStops.size() < fStopCount * 4 || ((fColorStops.size() - fStopCount * 4) % 2)) {
        SkDebugf("!! Invalid gradient stop array size: %zu", fColorStops.size());
        return;
    }

    std::vector<sksg::Gradient::ColorStop> stops;

    // TODO: merge/lerp opacity stops
    const auto csEnd = fColorStops.cbegin() + fStopCount * 4;
    for (auto cs = fColorStops.cbegin(); cs != csEnd; cs += 4) {
        stops.push_back({ *cs, VecToColor(&*(cs + 1), 3) });
    }

    fGradient->setColorStops(std::move(stops));
}

CompositeLinearGradient::CompositeLinearGradient(sk_sp<sksg::LinearGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void CompositeLinearGradient::onApply() {
    auto* grad = static_cast<sksg::LinearGradient*>(fGradient.get());
    grad->setStartPoint(this->startPoint());
    grad->setEndPoint(this->endPoint());
}

CompositeRadialGradient::CompositeRadialGradient(sk_sp<sksg::RadialGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void CompositeRadialGradient::onApply() {
    auto* grad = static_cast<sksg::RadialGradient*>(fGradient.get());
    grad->setStartCenter(this->startPoint());
    grad->setEndCenter(this->startPoint());
    grad->setStartRadius(0);
    grad->setEndRadius(SkPoint::Distance(this->startPoint(), this->endPoint()));
}

} // namespace skottie
