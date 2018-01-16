/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottiePriv_DEFINED
#define SkottiePriv_DEFINED

#include "SkJSONCPP.h"
#include "SkPoint.h"
#include "SkScalar.h"
#include "SkString.h"

namespace skottie {

#define LOG SkDebugf

static inline SkScalar ParseScalar(const Json::Value& v, SkScalar defaultValue) {
    return !v.isNull() && v.isConvertibleTo(Json::realValue)
        ? v.asFloat() : defaultValue;
}

static inline SkString ParseString(const Json::Value& v, const char defaultValue[]) {
    return SkString(!v.isNull() && v.isConvertibleTo(Json::stringValue)
                    ? v.asCString() : defaultValue);
}

static inline int ParseInt(const Json::Value& v, int defaultValue) {
    return !v.isNull() && v.isConvertibleTo(Json::intValue)
        ? v.asInt() : defaultValue;
}

static inline bool ParseBool(const Json::Value& v, bool defaultValue) {
    return !v.isNull() && v.isConvertibleTo(Json::booleanValue)
        ? v.asBool() : defaultValue;
}

static inline SkPoint ParsePoint(const Json::Value& v, const SkPoint& defaultValue) {
    return v.isObject()
        ? SkPoint::Make(ParseScalar(v["x"], defaultValue.x()),
                        ParseScalar(v["y"], defaultValue.y()))
        : defaultValue;
}

} // namespace

#endif // SkottiePriv_DEFINED
