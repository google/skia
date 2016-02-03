/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSwizzler_opts_DEFINED
#define SkSwizzler_opts_DEFINED

#include "SkColorPriv.h"

namespace SK_OPTS_NS {

// These variable names in these functions just pretend the input is BGRA.
// They work fine with both RGBA and BGRA.

static void premul_xxxa_portable(uint32_t dst[], const uint32_t src[], int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = src[i] >> 24,
                r = src[i] >> 16,
                g = src[i] >>  8,
                b = src[i] >>  0;
        r = (r*a+127)/255;
        g = (g*a+127)/255;
        b = (b*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)r << 16
               | (uint32_t)g <<  8
               | (uint32_t)b <<  0;
    }
}

static void premul_swaprb_xxxa_portable(uint32_t dst[], const uint32_t src[], int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = src[i] >> 24,
                r = src[i] >> 16,
                g = src[i] >>  8,
                b = src[i] >>  0;
        r = (r*a+127)/255;
        g = (g*a+127)/255;
        b = (b*a+127)/255;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)b << 16
               | (uint32_t)g <<  8
               | (uint32_t)r <<  0;
    }
}

#if defined(SK_ARM_HAS_NEON)

// Rounded divide by 255, (x + 127) / 255
static uint8x8_t div255_round(uint16x8_t x) {
    // result = (x + 127) / 255
    // result = (x + 127) / 256 + error1
    //
    // error1 = (x + 127) / (255 * 256)
    // error1 = (x + 127) / (256 * 256) + error2
    //
    // error2 = (x + 127) / (255 * 256 * 256)
    //
    // The maximum value of error2 is too small to matter.  Thus:
    // result = (x + 127) / 256 + (x + 127) / (256 * 256)
    // result = ((x + 127) / 256 + x + 127) / 256
    // result = ((x + 127) >> 8 + x + 127) >> 8
    //
    // Use >>> to represent "rounded right shift" which, conveniently,
    // NEON supports in one instruction.
    // result = ((x >>> 8) + x) >>> 8
    //
    // Note that the second right shift is actually performed as an
    // "add, round, and narrow back to 8-bits" instruction.
    return vraddhn_u16(x, vrshrq_n_u16(x, 8));
}

// Scale a byte by another, (x * y + 127) / 255
static uint8x8_t scale(uint8x8_t x, uint8x8_t y) {
    return div255_round(vmull_u8(x, y));
}

template <bool kSwapRB>
static void premul_xxxa_should_swaprb(uint32_t dst[], const uint32_t src[], int count) {
    while (count >= 8) {
        // Load 8 pixels.
        uint8x8x4_t bgra = vld4_u8((const uint8_t*) src);

        uint8x8_t a = bgra.val[3],
                  r = bgra.val[2],
                  g = bgra.val[1],
                  b = bgra.val[0];

        // Premultiply.
        r = scale(r, a);
        g = scale(g, a);
        b = scale(b, a);

        // Store 8 premultiplied pixels.
        if (kSwapRB) {
            bgra.val[2] = b;
            bgra.val[1] = g;
            bgra.val[0] = r;
        } else {
            bgra.val[2] = r;
            bgra.val[1] = g;
            bgra.val[0] = b;
        }
        vst4_u8((uint8_t*) dst, bgra);
        src += 8;
        dst += 8;
        count -= 8;
    }

    // Call portable code to finish up the tail of [0,8) pixels.
    auto proc = kSwapRB ? premul_swaprb_xxxa_portable : premul_xxxa_portable;
    proc(dst, src, count);
}

static void premul_xxxa(uint32_t dst[], const uint32_t src[], int count) {
    premul_xxxa_should_swaprb<false>(dst, src, count);
}

static void premul_swaprb_xxxa(uint32_t dst[], const uint32_t src[], int count) {
    premul_xxxa_should_swaprb<true>(dst, src, count);
}

#else

static void premul_xxxa(uint32_t dst[], const uint32_t src[], int count) {
    premul_xxxa_portable(dst, src, count);
}

static void premul_swaprb_xxxa(uint32_t dst[], const uint32_t src[], int count) {
    premul_swaprb_xxxa_portable(dst, src, count);
}

#endif

static void swaprb_xxxa(uint32_t dst[], const uint32_t src[], int count) {
    for (int i = 0; i < count; i++) {
        uint8_t a = src[i] >> 24,
                r = src[i] >> 16,
                g = src[i] >>  8,
                b = src[i] >>  0;
        dst[i] = (uint32_t)a << 24
               | (uint32_t)b << 16
               | (uint32_t)g <<  8
               | (uint32_t)r <<  0;
    }
}

}

#endif // SkSwizzler_opts_DEFINED
