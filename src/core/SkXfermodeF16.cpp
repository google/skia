/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkHalf.h"
#include "SkPM4fPriv.h"
#include "SkUtils.h"
#include "SkXfermode.h"

static Sk4f lerp_by_coverage(const Sk4f& src, const Sk4f& dst, uint8_t srcCoverage) {
    return dst + (src - dst) * Sk4f(srcCoverage * (1/255.0f));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void xfer_1(const SkXfermode* xfer, uint64_t dst[], const SkPM4f* src, int count,
                       const SkAlpha aa[]) {
    SkXfermodeProc4f proc = xfer->getProc4f();
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = SkHalfToFloat_01(dst[i]);
            d4.store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(*src, d).fVec);
            dst[i] = SkFloatToHalf_01(lerp_by_coverage(r4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            SkHalfToFloat_01(dst[i]).store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(*src, d).fVec);
            dst[i] = SkFloatToHalf_01(r4);
        }
    }
}

static void xfer_n(const SkXfermode* xfer, uint64_t dst[], const SkPM4f src[], int count,
                       const SkAlpha aa[]) {
    SkXfermodeProc4f proc = xfer->getProc4f();
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = SkHalfToFloat_01(dst[i]);
            d4.store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(src[i], d).fVec);
            dst[i] = SkFloatToHalf_01(lerp_by_coverage(r4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            SkHalfToFloat_01(dst[i]).store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(src[i], d).fVec);
            dst[i] = SkFloatToHalf_01(r4);
        }
    }
}

const SkXfermode::F16Proc gProcs_General[] = { xfer_n, xfer_n, xfer_1, xfer_1 };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void clear(const SkXfermode*, uint64_t dst[], const SkPM4f*, int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            if (aa[i]) {
                const Sk4f d4 = SkHalfToFloat_01(dst[i]);
                dst[i] = SkFloatToHalf_01(d4 * Sk4f((255 - aa[i]) * 1.0f/255));
            }
        }
    } else {
        sk_memset64(dst, 0, count);
    }
}

const SkXfermode::F16Proc gProcs_Clear[] = { clear, clear, clear, clear };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void src_1(const SkXfermode*, uint64_t dst[], const SkPM4f* src, int count,
                  const SkAlpha aa[]) {
    const Sk4f s4 = Sk4f::Load(src->fVec);
    if (aa) {
        for (int i = 0; i < count; ++i) {
            const Sk4f d4 = SkHalfToFloat_01(dst[i]);
            dst[i] = SkFloatToHalf_01(lerp_by_coverage(s4, d4, aa[i]));
        }
    } else {
        sk_memset64(dst, SkFloatToHalf_01(s4), count);
    }
}

static void src_n(const SkXfermode*, uint64_t dst[], const SkPM4f src[], int count,
                  const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            const Sk4f s4 = Sk4f::Load(src[i].fVec);
            const Sk4f d4 = SkHalfToFloat_01(dst[i]);
            dst[i] = SkFloatToHalf_01(lerp_by_coverage(s4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            const Sk4f s4 = Sk4f::Load(src[i].fVec);
            dst[i] = SkFloatToHalf_01(s4);
        }
    }
}

const SkXfermode::F16Proc gProcs_Src[] = { src_n, src_n, src_1,  src_1 };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void dst(const SkXfermode*, uint64_t*, const SkPM4f*, int count, const SkAlpha[]) {}

const SkXfermode::F16Proc gProcs_Dst[] = { dst, dst, dst, dst };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void srcover_1(const SkXfermode*, uint64_t dst[], const SkPM4f* src, int count,
                      const SkAlpha aa[]) {
    const Sk4f s4 = Sk4f::Load(src->fVec);
    const Sk4f dst_scale = Sk4f(1 - get_alpha(s4));
    for (int i = 0; i < count; ++i) {
        const Sk4f d4 = SkHalfToFloat_01(dst[i]);
        const Sk4f r4 = s4 + d4 * dst_scale;
        if (aa) {
            dst[i] = SkFloatToHalf_01(lerp_by_coverage(r4, d4, aa[i]));
        } else {
            dst[i] = SkFloatToHalf_01(r4);
        }
    }
}

static void srcover_n(const SkXfermode*, uint64_t dst[], const SkPM4f src[], int count,
                      const SkAlpha aa[]) {
    for (int i = 0; i < count; ++i) {
        Sk4f s = Sk4f::Load(src+i),
             d = SkHalfToFloat_01(dst+i),
             r = s + d*(1.0f - SkNx_shuffle<3,3,3,3>(s));
        if (aa) {
            r = lerp_by_coverage(r, d, aa[i]);
        }
        SkFloatToHalf_01(r, dst+i);
    }
}

const SkXfermode::F16Proc gProcs_SrcOver[] = { srcover_n, src_n, srcover_1, src_1 };

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkXfermode::F16Proc find_proc(SkXfermode::Mode mode, uint32_t flags) {
    SkASSERT(0 == (flags & ~3));
    flags &= 3;

    switch (mode) {
        case SkXfermode::kClear_Mode:   return gProcs_Clear[flags];
        case SkXfermode::kSrc_Mode:     return gProcs_Src[flags];
        case SkXfermode::kDst_Mode:     return gProcs_Dst[flags];
        case SkXfermode::kSrcOver_Mode: return gProcs_SrcOver[flags];
        default:
            break;
    }
    return gProcs_General[flags];
}

SkXfermode::F16Proc SkXfermode::onGetF16Proc(uint32_t flags) const {
    SkASSERT(0 == (flags & ~3));
    flags &= 3;

    Mode mode;
    return this->asMode(&mode) ? find_proc(mode, flags) : gProcs_General[flags];
}

SkXfermode::F16Proc SkXfermode::GetF16Proc(SkXfermode* xfer, uint32_t flags) {
    return xfer ? xfer->onGetF16Proc(flags) : find_proc(SkXfermode::kSrcOver_Mode, flags);
}
