/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieParser_DEFINED
#define SkottieParser_DEFINED

namespace Json { class Value; }

namespace skottie {

template <typename T>
bool Parse(const Json::Value&, T*);

template <typename T>
static inline T ParseDefault(const Json::Value& jv, const T& defaultValue) {
    T v;
    if (!Parse<T>(jv, &v))
        v = defaultValue;
    return v;
}

} // nasmespace skottie

#endif // SkottieParser_DEFINED
