/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk3D_DEFINED
#define Sk3D_DEFINED

#include "SkPoint3.h"
#include "SkMatrix44.h"

SK_API void Sk3LookAt(SkMatrix44* dst, const SkPoint3& eye, const SkPoint3& center, const SkPoint3& up);
SK_API bool Sk3Perspective(SkMatrix44* dst, float near, float far, float angle);
SK_API void Sk3MapPts(SkPoint dst[], const SkMatrix44& m4, const SkPoint3 src[], int count);

#endif

