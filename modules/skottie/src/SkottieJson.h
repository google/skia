/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieJson_DEFINED
#define SkottieJson_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/utils/SkJSON.h"

class SkData;
class SkStream;
class SkString;

namespace skottie {

template <typename T>
bool Parse(const skjson::Value&, T*);

template <typename T>
T ParseDefault(const skjson::Value& v, const T& defaultValue) {
    T res;
    if (!Parse<T>(v, &res)) {
        res = defaultValue;
    }
    return res;
}

} // namespace skottie

#endif // SkottieJson_DEFINED
