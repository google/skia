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

        vec->fVals.push_back(ScalarValue({ParseScalar(el, 0)}));
    }

    return true;
}

bool ShapeValue::Parse(const Json::Value& v, ShapeValue* shape) {
    PointArray inPts,
               outPts,
               verts;

    if (!v.isObject() ||
        !ParsePoints(v["i"], &inPts) ||
        !ParsePoints(v["o"], &outPts) ||
        !ParsePoints(v["v"], &verts) ||
        inPts.count() != outPts.count() ||
        inPts.count() != verts.count()) {

        LOG("!! could not parse shape: >%s<\n", v.toStyledString().c_str());
        return false;
    }

    SkASSERT(shape->fVertices.empty());
    for (int i = 0; i < inPts.count(); ++i) {
        shape->fVertices.emplace_back(BezierVertex({inPts[i], outPts[i], verts[i]}));
    }

    shape->fClose = ParseBool(v["c"], false);

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
SkPath ShapeValue::as<SkPath>() const {
    SkPath path;

    if (!fVertices.empty()) {
        path.moveTo(fVertices.front().fVertex);
    }

    const auto& addCubic = [](const BezierVertex& from, const BezierVertex& to, SkPath* path) {
        path->cubicTo(from.fVertex + from.fOutPoint,
                      to.fVertex   + to.fInPoint,
                      to.fVertex);
    };

    for (int i = 1; i < fVertices.count(); ++i) {
        addCubic(fVertices[i - 1], fVertices[i], &path);
    }

    if (fClose) {
        addCubic(fVertices.back(), fVertices.front(), &path);
    }

    return path;
}

template <>
ScalarValue Keyframed<ScalarValue>::Keyframe::lerp(float t) const {
    return ScalarValue({ fStartValue.as<SkScalar>() * (1 - t) + fEndValue.as<SkScalar>() * t });
}

template <>
VectorValue Keyframed<VectorValue>::Keyframe::lerp(float t) const {
    SkASSERT(fStartValue.fVals.count() == fEndValue.fVals.count());

    VectorValue result;
    result.fVals.reserve(fStartValue.fVals.count());

    for (int i = 0; i < fStartValue.fVals.count(); ++i) {
        const auto v0 = fStartValue.fVals[i].as<SkScalar>(),
                   v1 = fEndValue.fVals[i].as<SkScalar>();
        result.fVals.push_back(ScalarValue({ v0 * (1 - t) + v1 * t }));
    }

    return result;
}

template <>
ShapeValue Keyframed<ShapeValue>::Keyframe::lerp(float t) const {
    // TODO
    return fStartValue;
}

} // namespace skotty
