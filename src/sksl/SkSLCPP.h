/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CPP
#define SKSL_CPP

// functions used by CPP programs created by skslc

#include <cmath>
#include "SkPoint.h"

using std::abs;

// macros to make sk_Caps.<cap name> work from C++ code
#define sk_Caps (*args.fShaderCaps)

#define floatIs32Bits floatIs32Bits()

// functions to make GLSL constructors work from C++ code
inline SkPoint float2(float xy) { return SkPoint::Make(xy, xy); }

inline SkPoint float2(float x, float y) { return SkPoint::Make(x, y); }

inline SkRect float4(float ltrb) { return SkRect::MakeLTRB(ltrb, ltrb, ltrb, ltrb); }

inline SkRect float4(float l, float t, float r, float b) { return SkRect::MakeLTRB(l, t, r, b); }

#define half2 float2

#endif
