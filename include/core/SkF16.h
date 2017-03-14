/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkF16_DEFINED
#define SkF16_DEFINED

#include "SkTypes.h"

typedef uint16_t float16;

/**
 *  Converts half-precision floating point values in |src| to full-precision
 *  floating point valued stored in |dst|.
 *  Requires finite inputs, denormal values are flushed to zero.
 */
SK_API void SkF16ToF32(float* dst, const float16* src, int count);

/**
 *  Converts full-precision floating point values in |src| to half-precision
 *  floating point valued stored in |dst|.
 *  Requires finite inputs, denormal values are flushed to zero.
 */
SK_API void SkF32ToF16(float16* dst, const float* src, int count);

#endif
