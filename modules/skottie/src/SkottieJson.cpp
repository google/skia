/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieJson.h"

#include "include/core/SkData.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "modules/skottie/src/SkottieValue.h"
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

template <typename T>
bool ParseIntegral(const Value& v, T* result) {
    if (const skjson::NumberValue* num = v) {
        const auto dbl = **num;
        *result = static_cast<T>(dbl);
        return static_cast<double>(*result) == dbl;
    }

    return false;
}

template <>
bool Parse<int>(const Value& v, int* i) {
    return ParseIntegral(v, i);
}

template <>
bool Parse<size_t>(const Value& v, size_t* sz) {
    return ParseIntegral(v, sz);
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

} // namespace skottie
