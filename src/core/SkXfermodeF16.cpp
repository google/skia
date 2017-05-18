/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkHalf.h"
#include "SkPM4fPriv.h"
#include "SkUtils.h"
#include "SkXfermodePriv.h"

static Sk4f lerp_by_coverage(const Sk4f& src, const Sk4f& dst, uint8_t srcCoverage) {
    return dst + (src - dst) * Sk4f(srcCoverage * (1/255.0f));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void xfer_1(SkBlendMode mode, uint64_t dst[], const SkPM4f* src, int count,
                       const SkAlpha aa[]) {
    SkXfermodeProc4f proc = SkXfermode::GetProc4f(mode);
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = SkHalfToFloat_finite_ftz(dst[i]);
            d4.store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(*src, d).fVec);
            SkFloatToHalf_finite_ftz(lerp_by_coverage(r4, d4, aa[i])).store(&dst[i]);
        }
    } else {
        for (int i = 0; i < count; ++i) {
            SkHalfToFloat_finite_ftz(dst[i]).store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(*src, d).fVec);
            SkFloatToHalf_finite_ftz(r4).store(&dst[i]);
        }
    }
}

static void xfer_n(SkBlendMode mode, uint64_t dst[], const SkPM4f src[], int count,
                       const SkAlpha aa[]) {
    SkXfermodeProc4f proc = SkXfermode::GetProc4f(mode);
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = SkHalfToFloat_finite_ftz(dst[i]);
            d4.store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(src[i], d).fVec);
            SkFloatToHalf_finite_ftz(lerp_by_coverage(r4, d4, aa[i])).store(&dst[i]);
        }
    } else {
        for (int i = 0; i < count; ++i) {
            SkHalfToFloat_finite_ftz(dst[i]).store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(src[i], d).fVec);
            SkFloatToHalf_finite_ftz(r4).store(&dst[i]);
        }
    }
}

const SkXfermode::F16Proc gProcs_General[] = { xfer_n, xfer_n, xfer_1, xfer_1 };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void clear(SkBlendMode, uint64_t dst[], const SkPM4f*, int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            if (aa[i]) {
                const Sk4f d4 = SkHalfToFloat_finite_ftz(dst[i]);
                SkFloatToHalf_finite_ftz(d4 * Sk4f((255 - aa[i]) * 1.0f/255)).store(&dst[i]);
            }
        }
    } else {
        sk_memset64(dst, 0, count);
    }
}

const SkXfermode::F16Proc gProcs_Clear[] = { clear, clear, clear, clear };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void src_1(SkBlendMode, uint64_t dst[], const SkPM4f* src, int count, const SkAlpha aa[]) {
    const Sk4f s4 = Sk4f::Load(src->fVec);
    if (aa) {
        for (int i = 0; i < count; ++i) {
            const Sk4f d4 = SkHalfToFloat_finite_ftz(dst[i]);
            SkFloatToHalf_finite_ftz(lerp_by_coverage(s4, d4, aa[i])).store(&dst[i]);
        }
    } else {
        uint64_t s4h;
        SkFloatToHalf_finite_ftz(s4).store(&s4h);
        sk_memset64(dst, s4h, count);
    }
}

static void src_n(SkBlendMode, uint64_t dst[], const SkPM4f src[], int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            const Sk4f s4 = Sk4f::Load(src[i].fVec);
            const Sk4f d4 = SkHalfToFloat_finite_ftz(dst[i]);
            SkFloatToHalf_finite_ftz(lerp_by_coverage(s4, d4, aa[i])).store(&dst[i]);
        }
    } else {
        for (int i = 0; i < count; ++i) {
            const Sk4f s4 = Sk4f::Load(src[i].fVec);
            SkFloatToHalf_finite_ftz(s4).store(&dst[i]);
        }
    }
}

const SkXfermode::F16Proc gProcs_Src[] = { src_n, src_n, src_1,  src_1 };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void dst(SkBlendMode, uint64_t*, const SkPM4f*, int count, const SkAlpha[]) {}

const SkXfermode::F16Proc gProcs_Dst[] = { dst, dst, dst, dst };

///////////////////////////////////////////////////////////////////////////////////////////////////

static void srcover_1(SkBlendMode, uint64_t dst[], const SkPM4f* src, int count,
                      const SkAlpha aa[]) {
    const Sk4f s4 = Sk4f::Load(src->fVec);
    const Sk4f dst_scale = Sk4f(1 - get_alpha(s4));
    for (int i = 0; i < count; ++i) {
        const Sk4f d4 = SkHalfToFloat_finite_ftz(dst[i]);
        const Sk4f r4 = s4 + d4 * dst_scale;
        if (aa) {
            SkFloatToHalf_finite_ftz(lerp_by_coverage(r4, d4, aa[i])).store(&dst[i]);
        } else {
            SkFloatToHalf_finite_ftz(r4).store(&dst[i]);
        }
    }
}

static void srcover_n(SkBlendMode, uint64_t dst[], const SkPM4f src[], int count,
                      const SkAlpha aa[]) {
    for (int i = 0; i < count; ++i) {
        Sk4f s = Sk4f::Load(src+i),
             d = SkHalfToFloat_finite_ftz(dst[i]),
             r = s + d*(1.0f - SkNx_shuffle<3,3,3,3>(s));
        if (aa) {
            r = lerp_by_coverage(r, d, aa[i]);
        }
        SkFloatToHalf_finite_ftz(r).store(&dst[i]);
    }
}

const SkXfermode::F16Proc gProcs_SrcOver[] = { srcover_n, src_n, srcover_1, src_1 };

///////////////////////////////////////////////////////////////////////////////////////////////////

SkXfermode::F16Proc SkXfermode::GetF16Proc(SkBlendMode mode, uint32_t flags) {
    SkASSERT(0 == (flags & ~3));
    flags &= 3;

    switch (mode) {
        case SkBlendMode::kClear:   return gProcs_Clear[flags];
        case SkBlendMode::kSrc:     return gProcs_Src[flags];
        case SkBlendMode::kDst:     return gProcs_Dst[flags];
        case SkBlendMode::kSrcOver: return gProcs_SrcOver[flags];
        default:
            break;
    }
    return gProcs_General[flags];
}
