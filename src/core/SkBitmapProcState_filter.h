
/*
 * Copyright 2009 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkColorPriv.h"

/*
    Filter_32_opaque

    There is no hard-n-fast rule that the filtering must produce
    exact results for the color components, but if the 4 incoming colors are
    all opaque, then the output color must also be opaque. Subsequent parts of
    the drawing pipeline may rely on this (e.g. which blitrow proc to use).
 */

static inline void Filter_32_opaque(unsigned x, unsigned y,
                                    SkPMColor a00, SkPMColor a01,
                                    SkPMColor a10, SkPMColor a11,
                                    SkPMColor* dstColor) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);

    int xy = x * y;
    const uint32_t mask = 0xFF00FF;

    int scale = 256 - 16*y - 16*x + xy;
    uint32_t lo = (a00 & mask) * scale;
    uint32_t hi = ((a00 >> 8) & mask) * scale;

    scale = 16*x - xy;
    lo += (a01 & mask) * scale;
    hi += ((a01 >> 8) & mask) * scale;

    scale = 16*y - xy;
    lo += (a10 & mask) * scale;
    hi += ((a10 >> 8) & mask) * scale;

    lo += (a11 & mask) * xy;
    hi += ((a11 >> 8) & mask) * xy;

    *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
}

static inline void Filter_32_alpha(unsigned x, unsigned y,
                                   SkPMColor a00, SkPMColor a01,
                                   SkPMColor a10, SkPMColor a11,
                                   SkPMColor* dstColor,
                                   unsigned alphaScale) {
    SkASSERT((unsigned)x <= 0xF);
    SkASSERT((unsigned)y <= 0xF);
    SkASSERT(alphaScale <= 256);

    int xy = x * y;
    const uint32_t mask = 0xFF00FF;

    int scale = 256 - 16*y - 16*x + xy;
    uint32_t lo = (a00 & mask) * scale;
    uint32_t hi = ((a00 >> 8) & mask) * scale;

    scale = 16*x - xy;
    lo += (a01 & mask) * scale;
    hi += ((a01 >> 8) & mask) * scale;

    scale = 16*y - xy;
    lo += (a10 & mask) * scale;
    hi += ((a10 >> 8) & mask) * scale;

    lo += (a11 & mask) * xy;
    hi += ((a11 >> 8) & mask) * xy;

    lo = ((lo >> 8) & mask) * alphaScale;
    hi = ((hi >> 8) & mask) * alphaScale;

    *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
}

// Two color version, where we filter only along 1 axis
static inline void Filter_32_opaque(unsigned t,
                                    SkPMColor color0,
                                    SkPMColor color1,
                                    SkPMColor* dstColor) {
    SkASSERT((unsigned)t <= 0xF);

    const uint32_t mask = 0xFF00FF;

    int scale = 256 - 16*t;
    uint32_t lo = (color0 & mask) * scale;
    uint32_t hi = ((color0 >> 8) & mask) * scale;

    scale = 16*t;
    lo += (color1 & mask) * scale;
    hi += ((color1 >> 8) & mask) * scale;

    *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
}

// Two color version, where we filter only along 1 axis
static inline void Filter_32_alpha(unsigned t,
                                   SkPMColor color0,
                                   SkPMColor color1,
                                   SkPMColor* dstColor,
                                   unsigned alphaScale) {
    SkASSERT((unsigned)t <= 0xF);
    SkASSERT(alphaScale <= 256);

    const uint32_t mask = 0xFF00FF;

    int scale = 256 - 16*t;
    uint32_t lo = (color0 & mask) * scale;
    uint32_t hi = ((color0 >> 8) & mask) * scale;

    scale = 16*t;
    lo += (color1 & mask) * scale;
    hi += ((color1 >> 8) & mask) * scale;

    lo = ((lo >> 8) & mask) * alphaScale;
    hi = ((hi >> 8) & mask) * alphaScale;

    *dstColor = ((lo >> 8) & mask) | (hi & ~mask);
}
