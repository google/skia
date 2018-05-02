/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieJson.h"

#include "SkData.h"
#include "SkScalar.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkottieValue.h"

#include <vector>

namespace skottie {

namespace json {

template <>
bool json::ValueRef::as<SkScalar>(SkScalar* v) const {
    // Some versions wrap values as single-element arrays.
    if (fValue->isArray() && fValue->size() == 1) {
        return json::ValueRef(fValue->operator[](0)).as(v);
    }

    if (fValue->isNull() || !fValue->isConvertibleTo(Json::realValue))
        return false;

    *v = fValue->asFloat();

    return true;
}

template <>
bool json::ValueRef::as<bool>(bool* v) const {
    if (fValue->isNull() || !fValue->isConvertibleTo(Json::booleanValue))
        return false;

    *v = fValue->asBool();

    return true;
}

template <>
bool json::ValueRef::as<int>(int* v) const {
    if (fValue->isNull() || !fValue->isConvertibleTo(Json::intValue))
        return false;

    *v = fValue->asInt();

    return true;
}

template <>
bool json::ValueRef::as<SkString>(SkString* v) const {
    if (fValue->isNull() || !fValue->isConvertibleTo(Json::stringValue))
        return false;

    v->set(fValue->isString() ? fValue->asCString() : fValue->asString().c_str());

    return true;
}

template <>
bool json::ValueRef::as<SkPoint>(SkPoint* v) const {
    if (!fValue->isObject())
        return false;

    const auto& jvx = fValue->operator[]("x"),
                jvy = fValue->operator[]("y");

    // Some BM versions seem to store x/y as single-element arrays.
    return json::ValueRef(jvx.isArray() ? jvx[0] : jvx).as(&v->fX)
        && json::ValueRef(jvy.isArray() ? jvy[0] : jvy).as(&v->fY);
}

template <>
bool json::ValueRef::as<std::vector<float>>(std::vector<float>* v) const {
    if (!fValue->isArray())
        return false;

    v->resize(fValue->size());

    for (Json::ArrayIndex i = 0; i < fValue->size(); ++i) {
        if (!json::ValueRef(fValue->operator[](i)).as(v->data() + i)) {
            return false;
        }
    }

    return true;
}

namespace {

bool ParsePointVec(const Json::Value& jv, std::vector<SkPoint>* pts) {
    if (!jv.isArray())
        return false;

    pts->clear();
    pts->reserve(jv.size());

    std::vector<float> vec;
    for (Json::ArrayIndex i = 0; i < jv.size(); ++i) {
        if (!json::ValueRef(jv[i]).as(&vec) || vec.size() != 2)
            return false;
        pts->push_back(SkPoint::Make(vec[0], vec[1]));
    }

    return true;
}

} // namespace

template <>
bool json::ValueRef::as<ShapeValue>(ShapeValue* v) const {
    SkASSERT(v->fVertices.empty());

    // Some versions wrap values as single-element arrays.
    if (fValue->isArray() && fValue->size() == 1) {
        return json::ValueRef(fValue->operator[](0)).as(v);
    }

    std::vector<SkPoint> inPts,  // Cubic Bezier "in" control points, relative to vertices.
                         outPts, // Cubic Bezier "out" control points, relative to vertices.
                         verts;  // Cubic Bezier vertices.

    if (!fValue->isObject() ||
        !ParsePointVec(fValue->operator[]("i"), &inPts) ||
        !ParsePointVec(fValue->operator[]("o"), &outPts) ||
        !ParsePointVec(fValue->operator[]("v"), &verts) ||
        inPts.size() != outPts.size() ||
        inPts.size() != verts.size()) {

        return false;
    }

    v->fVertices.reserve(inPts.size());
    for (size_t i = 0; i < inPts.size(); ++i) {
        v->fVertices.push_back(BezierVertex({inPts[i], outPts[i], verts[i]}));
    }
    v->fClosed = json::ValueRef(fValue->operator[]("c")).asDefault<bool>(false);

    return true;
}

SkString json::ValueRef::toString() const {
    return SkString(fValue->toStyledString().c_str());
}

Json::Value ParseStream(SkStream* stream) {
    Json::Value json;

    if (stream->hasLength()) {
        if (auto data = SkData::MakeFromStream(stream, stream->getLength())) {
            Json::Reader reader;
            const auto dataStart = static_cast<const char*>(data->data());
            if (!reader.parse(dataStart, dataStart + data->size(), json, false)) {
                SkDebugf("!! failed to parse json: %s\n",
                         reader.getFormattedErrorMessages().c_str());
            }
        }
    }

    return json;
}

} // namespace json

} // namespace skottie
