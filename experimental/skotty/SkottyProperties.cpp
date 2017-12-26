/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottyProperties.h"

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

template <>
bool Property<float>::ParseValue(const Json::Value& v, float* val) {
    if (v.isNull() || !v.isConvertibleTo(Json::realValue))
        return false;

    *val = ParseScalar(v, 0);
    return true;
}

template <>
bool Property<SkTArray<float, true>>::ParseValue(const Json::Value& v,
                                                 SkTArray<float, true>* val) {
    SkASSERT(val->empty());

    if (!v.isArray())
        return false;

    for (Json::ArrayIndex i = 0; i < v.size(); ++i) {
        const auto& el = v[i];
        if (el.isNull() || !el.isConvertibleTo(Json::realValue))
            return false;

        val->push_back(ParseScalar(el, 0));
    }

    return true;
}

template <>
bool Property<ShapeValue>::ParseValue(const Json::Value& v, ShapeValue* shape) {
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

SkPath ShapeValue::asPath() const {
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

} // namespace skotty
