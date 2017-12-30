/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyPriv_DEFINED
#define SkottyPriv_DEFINED

#include "SkJSONCPP.h"
#include "SkScalar.h"
#include "SkString.h"

namespace skotty {

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

} // namespace

#endif // SkottyPriv_DEFINED
