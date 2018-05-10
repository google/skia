/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

static const union {
    uint32_t bits;
    float    f;
} inf_ = { 0x7f800000 };

#define INFINITY_ inf_.f

static inline float floorf_(float x) {
    float roundtrip = (float)((int)x);
    return roundtrip > x ? roundtrip - 1 : roundtrip;
}

static inline float fmaxf_(float x, float y) { return x > y ? x : y; }
static inline float fminf_(float x, float y) { return x < y ? x : y; }
static inline float fabsf_(float x) { return x < 0 ? -x : x; }

float log2f_(float);
float exp2f_(float);
float powf_(float, float);

static inline bool isfinitef_(float x) { return 0 == x*0; }
