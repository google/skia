/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottieJson.h"

#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/src/SkottieValue.h"
#include "src/utils/SkJSON.h"

#include <cstddef>
#include <limits>

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
        if (dbl > static_cast<double>(std::numeric_limits<T>::max()) ||
            dbl < static_cast<double>(std::numeric_limits<T>::min())) {
            return false;
        }

        *result = static_cast<T>(dbl);
        return true;
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
bool Parse<SkV2>(const Value& v, SkV2* v2) {
    if (!v.is<ArrayValue>())
        return false;
    const auto& av = v.as<ArrayValue>();

    // We need at least two scalars (BM sometimes exports a third value == 0).
    return av.size() >= 2
        && Parse<SkScalar>(av[0], &v2->x)
        && Parse<SkScalar>(av[1], &v2->y);
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
bool Parse<VectorValue>(const Value& v, VectorValue* vec) {
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

const skjson::StringValue* ParseSlotID(const skjson::ObjectValue* jobj) {
    if (jobj) {
        if (const skjson::StringValue* sid = (*jobj)["sid"]) {
            return sid;
        }
    }
    return nullptr;
}

} // namespace skottie
