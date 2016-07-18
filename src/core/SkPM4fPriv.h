/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM4fPriv_DEFINED
#define SkPM4fPriv_DEFINED

#include "SkColorPriv.h"
#include "SkPM4f.h"

static inline float get_alpha(const Sk4f& f4) {
    return f4[SkPM4f::A];
}

static inline Sk4f set_alpha(const Sk4f& f4, float alpha) {
    static_assert(3 == SkPM4f::A, "");
    return Sk4f(f4[0], f4[1], f4[2], alpha);
}

static inline uint32_t to_4b(const Sk4f& f4) {
    uint32_t b4;
    SkNx_cast<uint8_t>(f4).store((uint8_t*)&b4);
    return b4;
}

static inline Sk4f to_4f(uint32_t b4) {
    return SkNx_cast<float>(Sk4b::Load((const uint8_t*)&b4));
}

static inline Sk4f to_4f_rgba(uint32_t b4) {
    return swizzle_rb_if_bgra(to_4f(b4));
}

static inline Sk4f srgb_to_linear(const Sk4f& s4) {
    return set_alpha(s4 * s4, get_alpha(s4));
}

static inline Sk4f linear_to_srgb(const Sk4f& l4) {
    return set_alpha(l4.rsqrt().invert(), get_alpha(l4));
}

static inline float srgb_to_linear(float x) {
    return x * x;
}

static inline float linear_to_srgb(float x) {
    return sqrtf(x);
}

static void assert_unit(float x) {
    SkASSERT(x >= 0 && x <= 1);
}

static inline float exact_srgb_to_linear(float x) {
    assert_unit(x);
    float linear;
    if (x <= 0.04045) {
        linear = x / 12.92f;
    } else {
        linear = powf((x + 0.055f) / 1.055f, 2.4f);
    }
    assert_unit(linear);
    return linear;
}

static inline float exact_linear_to_srgb(float x) {
    assert_unit(x);
    float srgb;
    if (x <= 0.0031308f) {
        srgb = x * 12.92f;
    } else {
        srgb = 1.055f * powf(x, 0.41666667f) - 0.055f;
    }
    assert_unit(srgb);
    return srgb;
}

static inline Sk4f exact_srgb_to_linear(const Sk4f& x) {
    Sk4f linear(exact_srgb_to_linear(x[0]),
                exact_srgb_to_linear(x[1]),
                exact_srgb_to_linear(x[2]), 1);
    return set_alpha(linear, get_alpha(x));
}

static inline Sk4f exact_linear_to_srgb(const Sk4f& x) {
    Sk4f srgb(exact_linear_to_srgb(x[0]),
              exact_linear_to_srgb(x[1]),
              exact_linear_to_srgb(x[2]), 1);
    return set_alpha(srgb, get_alpha(x));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static inline Sk4f Sk4f_fromL32(uint32_t src) {
    return to_4f(src) * Sk4f(1.0f/255);
}

static inline Sk4f Sk4f_fromS32(uint32_t src) {
    return srgb_to_linear(to_4f(src) * Sk4f(1.0f/255));
}

// Color handling:
//   SkColor has an ordering of (b, g, r, a) if cast to an Sk4f, so the code swizzles r and b to
// produce the needed (r, g, b, a) ordering.
static inline Sk4f Sk4f_from_SkColor(SkColor color) {
    return swizzle_rb(Sk4f_fromS32(color));
}

static inline uint32_t Sk4f_toL32(const Sk4f& x4) {
    return to_4b(x4 * Sk4f(255) + Sk4f(0.5f));
}

static inline uint32_t Sk4f_toS32(const Sk4f& x4) {
    return to_4b(linear_to_srgb(x4) * Sk4f(255) + Sk4f(0.5f));
}

static inline Sk4f exact_Sk4f_fromS32(uint32_t src) {
    return exact_srgb_to_linear(to_4f(src) * Sk4f(1.0f/255));
}
static inline uint32_t exact_Sk4f_toS32(const Sk4f& x4) {
    return to_4b(exact_linear_to_srgb(x4) * Sk4f(255) + Sk4f(0.5f));
}

#endif
