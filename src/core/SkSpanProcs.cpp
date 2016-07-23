/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorFilter.h"
#include "SkHalf.h"
#include "SkNx.h"
#include "SkPaint.h"
#include "SkPixmap.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkSpanProcs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

static void load_l32(const SkPixmap& src, int x, int y, SkPM4f span[], int count) {
    SkASSERT(count > 0);
    const uint32_t* addr = src.addr32(x, y);
    SkASSERT(src.addr32(x + count - 1, y));

    for (int i = 0; i < count; ++i) {
        (to_4f_rgba(addr[i]) * Sk4f(1.0f/255)).store(span[i].fVec);
    }
}

static void load_s32(const SkPixmap& src, int x, int y, SkPM4f span[], int count) {
    SkASSERT(count > 0);
    const uint32_t* addr = src.addr32(x, y);
    SkASSERT(src.addr32(x + count - 1, y));

    for (int i = 0; i < count; ++i) {
        srgb_to_linear(to_4f_rgba(addr[i]) * Sk4f(1.0f/255)).store(span[i].fVec);
    }
}

static void load_f16(const SkPixmap& src, int x, int y, SkPM4f span[], int count) {
    SkASSERT(count > 0);
    const uint64_t* addr = src.addr64(x, y);
    SkASSERT(src.addr64(x + count - 1, y));

    for (int i = 0; i < count; ++i) {
        SkHalfToFloat_01(addr[i]).store(span[i].fVec);
    }
}

SkLoadSpanProc SkLoadSpanProc_Choose(const SkImageInfo& info) {
    switch (info.colorType()) {
        case kN32_SkColorType:
            return info.gammaCloseToSRGB() ? load_s32 : load_l32;
        case kRGBA_F16_SkColorType:
            return load_f16;
        default:
            return nullptr;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void noop_filterspan(const SkPaint& paint, SkPM4f[], int) {
    SkASSERT(!paint.getColorFilter());
    SkASSERT(0xFF == paint.getAlpha());
}

static void alpha_filterspan(const SkPaint& paint, SkPM4f span[], int count) {
    SkASSERT(!paint.getColorFilter());
    SkASSERT(0xFF != paint.getAlpha());
    const Sk4f scale = Sk4f(paint.getAlpha() * (1.0f/255));
    for (int i = 0; i < count; ++i) {
        (Sk4f::Load(span[i].fVec) * scale).store(span[i].fVec);
    }
}

static void colorfilter_filterspan(const SkPaint& paint, SkPM4f span[], int count) {
    SkASSERT(paint.getColorFilter());
    SkASSERT(0xFF == paint.getAlpha());
    paint.getColorFilter()->filterSpan4f(span, count, span);
}

static void colorfilter_alpha_filterspan(const SkPaint& paint, SkPM4f span[], int count) {
    SkASSERT(paint.getColorFilter());
    SkASSERT(0xFF != paint.getAlpha());
    alpha_filterspan(paint, span, count);
    paint.getColorFilter()->filterSpan4f(span, count, span);
}

SkFilterSpanProc SkFilterSpanProc_Choose(const SkPaint& paint) {
    if (paint.getColorFilter()) {
        return 0xFF == paint.getAlpha() ? colorfilter_filterspan : colorfilter_alpha_filterspan;
    } else {
        return 0xFF == paint.getAlpha() ? noop_filterspan : alpha_filterspan;
    }
}
