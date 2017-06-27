/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_CPP
#define SKSL_CPP

// functions used by CPP programs created by skslc

#include "SkPoint.h"

// macros to make sk_Caps.<cap name> work from C++ code
#define sk_Caps (*args.fShaderCaps)

#define floatPrecisionVaries floatPrecisionVaries()

// functions to make GLSL constructors work from C++ code
inline SkPoint vec2(float xy) { return SkPoint::Make(xy, xy); }

inline SkPoint vec2(float x, float y) { return SkPoint::Make(x, y); }

#endif
