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

using namespace skjson;

template <>
bool Parse<SkScalar>(const Value& v, SkScalar* s) {
    // Some versions wrap values as single-element arrays.
    if (const skjson::ArrayValue* array = v) {
        if (array->size() > 0) {
            return Parse((*array)[0], s);
        }
    }

    if (const skjson::NumberValue* num = v) {
        *s = static_cast<SkScalar>(**num);
        return true;
    }

    return false;
}

template <>
bool Parse<bool>(const Value& v, bool* b) {
    switch(v.getType()) {
    case Value::Type::kNumber:
        *b = SkToBool(*v.as<NumberValue>());
        return true;
    case Value::Type::kBool:
        *b = *v.as<BoolValue>();
        return true;
    default:
        break;
    }

    return false;
}

template <>
bool Parse<int>(const Value& v, int* i) {
    if (const skjson::NumberValue* num = v) {
        const auto dbl = **num;
        *i = dbl;
        return *i == dbl;
    }

    return false;
}

template <>
bool Parse<SkString>(const Value& v, SkString* s) {
    if (const skjson::StringValue* sv = v) {
        s->set(sv->begin(), sv->size());
        return true;
    }

    return false;
}

template <>
bool Parse<SkPoint>(const Value& v, SkPoint* pt) {
    if (!v.is<ObjectValue>())
        return false;
    const auto& ov = v.as<ObjectValue>();

    return Parse<SkScalar>(ov["x"], &pt->fX)
        && Parse<SkScalar>(ov["y"], &pt->fY);
}

template <>
bool Parse<std::vector<float>>(const Value& v, std::vector<float>* vec) {
    if (!v.is<ArrayValue>())
        return false;
    const auto& av = v.as<ArrayValue>();

    vec->resize(av.size());
    for (size_t i = 0; i < av.size(); ++i) {
        if (!Parse(av[i], vec->data() + i)) {
            return false;
        }
    }

    return true;
}

namespace {

bool ParsePointVec(const Value& v, std::vector<SkPoint>* pts) {
    if (!v.is<ArrayValue>())
        return false;
    const auto& av = v.as<ArrayValue>();

    pts->clear();
    pts->reserve(av.size());

    std::vector<float> vec;
    for (size_t i = 0; i < av.size(); ++i) {
        if (!Parse(av[i], &vec) || vec.size() != 2)
            return false;
        pts->push_back(SkPoint::Make(vec[0], vec[1]));
    }

    return true;
}

} // namespace

template <>
bool Parse<ShapeValue>(const Value& v, ShapeValue* sh) {
    SkASSERT(sh->fVertices.empty());

    // Some versions wrap values as single-element arrays.
    if (const skjson::ArrayValue* av = v) {
        if (av->size() == 1) {
            return Parse((*av)[0], sh);
        }
    }

    if (!v.is<skjson::ObjectValue>())
        return false;
    const auto& ov = v.as<ObjectValue>();

    std::vector<SkPoint> inPts,  // Cubic Bezier "in" control points, relative to vertices.
                         outPts, // Cubic Bezier "out" control points, relative to vertices.
                         verts;  // Cubic Bezier vertices.

    if (!ParsePointVec(ov["i"], &inPts) ||
        !ParsePointVec(ov["o"], &outPts) ||
        !ParsePointVec(ov["v"], &verts) ||
        inPts.size() != outPts.size() ||
        inPts.size() != verts.size()) {

        return false;
    }

    sh->fVertices.reserve(inPts.size());
    for (size_t i = 0; i < inPts.size(); ++i) {
        sh->fVertices.push_back(BezierVertex({inPts[i], outPts[i], verts[i]}));
    }
    sh->fClosed = ParseDefault<bool>(ov["c"], false);

    return true;
}

} // namespace skottie
