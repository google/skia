/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottyProperties.h"

#include "SkColor.h"
#include "SkottyPriv.h"
#include "SkPath.h"
#include "SkSGRect.h"
#include "SkSGTransform.h"

namespace  skotty {

namespace {

using PointArray = SkSTArray<64, SkPoint, true>;

bool ParsePoints(const Json::Value& v, PointArray* pts) {
    if (!v.isArray()) {
        return false;
    }

    for (Json::ArrayIndex i = 0; i < v.size(); ++i) {
        const auto& pt = v[i];
        if (!pt.isArray() || pt.size() != 2 ||
            !pt[0].isConvertibleTo(Json::realValue) ||
            !pt[1].isConvertibleTo(Json::realValue)) {
            return false;
        }

        pts->push_back(SkPoint::Make(ParseScalar(pt[0], 0), ParseScalar(pt[1], 0)));
    }
    return true;
}

} // namespace

bool ScalarValue::Parse(const Json::Value& v, ScalarValue* scalar) {
    // Some files appear to wrap keyframes in arrays for no reason.
    if (v.isArray() && v.size() == 1) {
        return Parse(v[0], scalar);
    }

    if (v.isNull() || !v.isConvertibleTo(Json::realValue))
        return false;

    scalar->fVal = ParseScalar(v, 0);
    return true;
}

bool VectorValue::Parse(const Json::Value& v, VectorValue* vec) {
    SkASSERT(vec->fVals.empty());

    if (!v.isArray())
        return false;

    for (Json::ArrayIndex i = 0; i < v.size(); ++i) {
        const auto& el = v[i];
        if (el.isNull() || !el.isConvertibleTo(Json::realValue))
            return false;

        vec->fVals.emplace_back(ParseScalar(el, 0));
    }

    return true;
}

bool ShapeValue::Parse(const Json::Value& v, ShapeValue* shape) {
    PointArray inPts,  // Cubic Bezier "in" control points, relative to vertices.
               outPts, // Cubic Bezier "out" control points, relative to vertices.
               verts;  // Cubic Bezier vertices.

    // Some files appear to wrap keyframes in arrays for no reason.
    if (v.isArray() && v.size() == 1) {
        return Parse(v[0], shape);
    }

    if (!v.isObject() ||
        !ParsePoints(v["i"], &inPts) ||
        !ParsePoints(v["o"], &outPts) ||
        !ParsePoints(v["v"], &verts) ||
        inPts.count() != outPts.count() ||
        inPts.count() != verts.count()) {

        return false;
    }

    SkASSERT(shape->fPath.isEmpty());

    if (!verts.empty()) {
        shape->fPath.moveTo(verts.front());
    }

    const auto& addCubic = [&](int from, int to) {
        shape->fPath.cubicTo(verts[from] + outPts[from],
                             verts[to]   + inPts[to],
                             verts[to]);
    };

    for (int i = 1; i < verts.count(); ++i) {
        addCubic(i - 1, i);
    }

    if (!verts.empty() && ParseBool(v["c"], false)) {
        addCubic(verts.count() - 1, 0);
        shape->fPath.close();
    }

    return true;
}

template <>
SkColor VectorValue::as<SkColor>() const {
    // best effort to turn this into a color
    const auto r = fVals.count() > 0 ? fVals[0].as<SkScalar>() : 0,
               g = fVals.count() > 1 ? fVals[1].as<SkScalar>() : 0,
               b = fVals.count() > 2 ? fVals[2].as<SkScalar>() : 0,
               a = fVals.count() > 3 ? fVals[3].as<SkScalar>() : 1;

    return SkColorSetARGB(SkTPin<SkScalar>(a, 0, 1) * 255,
                          SkTPin<SkScalar>(r, 0, 1) * 255,
                          SkTPin<SkScalar>(g, 0, 1) * 255,
                          SkTPin<SkScalar>(b, 0, 1) * 255);
}

template <>
SkPoint VectorValue::as<SkPoint>() const {
    // best effort to turn this into a point
    const auto x = fVals.count() > 0 ? fVals[0].as<SkScalar>() : 0,
               y = fVals.count() > 1 ? fVals[1].as<SkScalar>() : 0;
    return SkPoint::Make(x, y);
}

template <>
SkSize VectorValue::as<SkSize>() const {
    const auto pt = this->as<SkPoint>();
    return SkSize::Make(pt.x(), pt.y());
}

template <>
std::vector<SkScalar> VectorValue::as<std::vector<SkScalar>>() const {
    std::vector<SkScalar> vec;
    vec.reserve(fVals.count());

    for (const auto& val : fVals) {
        vec.push_back(val);
    }

    return vec;
}

template <>
SkPath ShapeValue::as<SkPath>() const {
    return fPath;
}

CompositeRRect::CompositeRRect(sk_sp<sksg::RRect> wrapped_node)
    : fRRectNode(std::move(wrapped_node)) {}

void CompositeRRect::apply() {
    // BM "position" == "center position"
    auto rr = SkRRect::MakeRectXY(SkRect::MakeXYWH(fPosition.x() - fSize.width() / 2,
                                                   fPosition.y() - fSize.height() / 2,
                                                   fSize.width(), fSize.height()),
                                  fRadius,
                                  fRadius);
   fRRectNode->setRRect(rr);
}

CompositeTransform::CompositeTransform(sk_sp<sksg::RenderNode> wrapped_node)
    : fTransformNode(sksg::Transform::Make(std::move(wrapped_node), SkMatrix::I())) {}

void CompositeTransform::apply() {
    SkMatrix t = SkMatrix::MakeTrans(-fAnchorPoint.x(), -fAnchorPoint.y());

    t.postScale(fScale.x() / 100, fScale.y() / 100); // 100% based
    t.postRotate(fRotation);
    t.postTranslate(fPosition.x(), fPosition.y());
    // TODO: skew

    fTransformNode->setMatrix(t);
}

} // namespace skotty
