/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk3D_DEFINED
#define Sk3D_DEFINED

#include "include/core/SkMatrix44.h"
#include "include/core/SkPoint3.h"

SK_API void Sk3LookAt(SkMatrix44* dst, const SkPoint3& eye, const SkPoint3& center, const SkPoint3& up);
static inline SkMatrix44 Sk3LookAt(const SkPoint3& eye, const SkPoint3& center, const SkPoint3& up) {
    SkMatrix44 m;
    Sk3LookAt(&m, eye, center, up);
    return m;
}

SK_API bool Sk3Perspective(SkMatrix44* dst, float near, float far, float angle);
static inline SkMatrix44 Sk3Perspective(float near, float far, float angle) {
    SkMatrix44 m;
    Sk3Perspective(&m, near, far, angle);
    return m;
}

SK_API void Sk3MapPts(SkPoint dst[], const SkMatrix44& m4, const SkPoint3 src[], int count);

#endif

