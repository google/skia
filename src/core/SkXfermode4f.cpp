/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPM4fPriv.h"
#include "SkUtils.h"
#include "SkXfermode.h"

struct XferProcPair {
    SkXfermode::PM4fProc1 fP1;
    SkXfermode::PM4fProcN fPN;
};

enum DstType {
    kLinear_Dst,
    kSRGB_Dst,
};

static Sk4f scale_by_coverage(const Sk4f& x4, uint8_t coverage) {
    return x4 * Sk4f(coverage * (1/255.0f));
}

static Sk4f lerp(const Sk4f& src, const Sk4f& dst, uint8_t srcCoverage) {
    return dst + (src - dst) * Sk4f(srcCoverage * (1/255.0f));
}

template <DstType D> Sk4f load_dst(SkPMColor dstC) {
    return (D == kSRGB_Dst) ? Sk4f_fromS32(dstC) : Sk4f_fromL32(dstC);
}

static Sk4f srgb_4b_to_linear_unit(SkPMColor dstC) {
    return Sk4f_fromS32(dstC);
}

template <DstType D> uint32_t store_dst(const Sk4f& x4) {
    return (D == kSRGB_Dst) ? Sk4f_toS32(x4) : Sk4f_toL32(x4);
}

static uint32_t linear_unit_to_srgb_32(const Sk4f& l4) {
    return Sk4f_toL32(l4);
}

static Sk4f linear_unit_to_srgb_255f(const Sk4f& l4) {
    return linear_to_srgb(l4) * Sk4f(255) + Sk4f(0.5f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static Sk4f scale_255_round(const SkPM4f& pm4) {
    return Sk4f::Load(pm4.fVec) * Sk4f(255) + Sk4f(0.5f);
}

static void pm4f_to_linear_32(SkPMColor dst[], const SkPM4f src[], int count) {
    while (count >= 4) {
        src[0].assertIsUnit();
        src[1].assertIsUnit();
        src[2].assertIsUnit();
        src[3].assertIsUnit();
        Sk4f_ToBytes((uint8_t*)dst,
                     scale_255_round(src[0]), scale_255_round(src[1]),
                     scale_255_round(src[2]), scale_255_round(src[3]));
        src += 4;
        dst += 4;
        count -= 4;
    }
    for (int i = 0; i < count; ++i) {
        src[i].assertIsUnit();
        SkNx_cast<uint8_t>(scale_255_round(src[i])).store((uint8_t*)&dst[i]);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// These are our fallback impl for the SkPM4f procs...
//
// They just convert the src color(s) into a linear SkPMColor value(s), and then
// call the existing virtual xfer32. This clear throws away data (converting floats to bytes)
// in the src, and ignores the sRGB flag, but should draw about the same as if the caller
// had passed in SkPMColor values directly.
//

void xfer_pm4_proc_1(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f& src,
                     int count, const SkAlpha aa[]) {
    uint32_t pm;
    pm4f_to_linear_32(&pm, &src, 1);

    const int N = 128;
    SkPMColor tmp[N];
    sk_memset32(tmp, pm, SkMin32(count, N));
    while (count > 0) {
        const int n = SkMin32(count, N);
        state.fXfer->xfer32(dst, tmp, n, aa);

        dst += n;
        if (aa) {
            aa += n;
        }
        count -= n;
    }
}

void xfer_pm4_proc_n(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f src[],
                     int count, const SkAlpha aa[]) {
    const int N = 128;
    SkPMColor tmp[N];
    while (count > 0) {
        const int n = SkMin32(count, N);
        pm4f_to_linear_32(tmp, src, n);
        state.fXfer->xfer32(dst, tmp, n, aa);

        src += n;
        dst += n;
        if (aa) {
            aa += n;
        }
        count -= n;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void clear_linear_n(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f[],
                           int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (a) {
                SkPMColor dstC = dst[i];
                SkPMColor C = 0;
                if (0xFF != a) {
                    C = SkFourByteInterp(C, dstC, a);
                }
                dst[i] = C;
            }
        }
    } else {
        sk_bzero(dst, count * sizeof(SkPMColor));
    }
}

static void clear_linear_1(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f&,
                           int count, const SkAlpha coverage[]) {
    clear_linear_n(state, dst, nullptr, count, coverage);
}

static void clear_srgb_n(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f[],
                           int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (a) {
                Sk4f d = Sk4f_fromS32(dst[i]) * Sk4f((255 - a) * (1/255.0f));
                dst[i] = Sk4f_toS32(d);
            }
        }
    } else {
        sk_bzero(dst, count * sizeof(SkPMColor));
    }
}

static void clear_srgb_1(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f&,
                           int count, const SkAlpha coverage[]) {
    clear_srgb_n(state, dst, nullptr, count, coverage);
}

const XferProcPair gProcs_Clear[] = {
    { clear_linear_1, clear_linear_n },       // linear   [alpha]
    { clear_linear_1, clear_linear_n },       // linear   [opaque]
    { clear_srgb_1,   clear_srgb_n   },       // srgb     [alpha]
    { clear_srgb_1,   clear_srgb_n   },       // srgb     [opaque]
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void src_n(const SkXfermode::PM4fState& state, uint32_t dst[],
                                const SkPM4f src[], int count, const SkAlpha aa[]) {
    for (int i = 0; i < count; ++i) {
        unsigned a = 0xFF;
        if (aa) {
            a = aa[i];
            if (0 == a) {
                continue;
            }
        }
        Sk4f r4 = Sk4f::Load(src[i].fVec);   // src always overrides dst
        if (a != 0xFF) {
            Sk4f d4 = load_dst<D>(dst[i]);
            r4 = lerp(r4, d4, a);
        }
        dst[i] = store_dst<D>(r4);
    }
}

template <DstType D> void src_1(const SkXfermode::PM4fState& state, uint32_t dst[],
                                const SkPM4f& src, int count, const SkAlpha aa[]) {
    const Sk4f r4 = Sk4f::Load(src.fVec);   // src always overrides dst
    const uint32_t r32 = store_dst<D>(r4);

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }
            if (a != 0xFF) {
                Sk4f d4 = load_dst<D>(dst[i]);
                dst[i] = store_dst<D>(lerp(r4, d4, a));
            } else {
                dst[i] = r32;
            }
        }
    } else {
        sk_memset32(dst, r32, count);
    }
}

const XferProcPair gProcs_Src[] = {
    { src_1<kLinear_Dst>, src_n<kLinear_Dst> },       // linear   [alpha]
    { src_1<kLinear_Dst>, src_n<kLinear_Dst> },       // linear   [opaque]
    { src_1<kSRGB_Dst>,   src_n<kSRGB_Dst>   },       // srgb     [alpha]
    { src_1<kSRGB_Dst>,   src_n<kSRGB_Dst>   },       // srgb     [opaque]
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void dst_n(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f[],
                         int count, const SkAlpha aa[]) {}

static void dst_1(const SkXfermode::PM4fState& state, uint32_t dst[], const SkPM4f&,
                  int count, const SkAlpha coverage[]) {}

const XferProcPair gProcs_Dst[] = {
    { dst_1, dst_n },
    { dst_1, dst_n },
    { dst_1, dst_n },
    { dst_1, dst_n },
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void srcover_n(const SkXfermode::PM4fState& state, uint32_t dst[],
                                    const SkPM4f src[], int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }
            Sk4f s4 = Sk4f::Load(src[i].fVec);
            Sk4f d4 = load_dst<D>(dst[i]);
            if (a != 0xFF) {
                s4 = scale_by_coverage(s4, a);
            }
            Sk4f r4 = s4 + d4 * Sk4f(1 - get_alpha(s4));
            dst[i] = store_dst<D>(r4);
        }
    } else {
        for (int i = 0; i < count; ++i) {
            Sk4f s4 = Sk4f::Load(src[i].fVec);
            Sk4f d4 = load_dst<D>(dst[i]);
            Sk4f r4 = s4 + d4 * Sk4f(1 - get_alpha(s4));
            dst[i] = store_dst<D>(r4);
        }
    }
}

static void srcover_linear_dst_1(const SkXfermode::PM4fState& state, uint32_t dst[],
                                 const SkPM4f& src, int count, const SkAlpha aa[]) {
    Sk4f s4 = Sk4f::Load(src.fVec);
    Sk4f dst_scale = Sk4f(1 - get_alpha(s4));
    
    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }
            Sk4f d4 = Sk4f_fromL32(dst[i]);
            Sk4f r4;
            if (a != 0xFF) {
                s4 = scale_by_coverage(s4, a);
                r4 = s4 + d4 * Sk4f(1 - get_alpha(s4));
            } else {
                r4 = s4 + d4 * dst_scale;
            }
            dst[i] = Sk4f_toL32(r4);
        }
    } else {
        s4 = s4 * Sk4f(255) + Sk4f(0.5f);   // +0.5 to pre-bias for rounding
        while (count >= 4) {
            Sk4f d0 = to_4f(dst[0]);
            Sk4f d1 = to_4f(dst[1]);
            Sk4f d2 = to_4f(dst[2]);
            Sk4f d3 = to_4f(dst[3]);
            Sk4f_ToBytes((uint8_t*)dst,
                         s4 + d0 * dst_scale,
                         s4 + d1 * dst_scale,
                         s4 + d2 * dst_scale,
                         s4 + d3 * dst_scale);
            dst += 4;
            count -= 4;
        }
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = to_4f(dst[i]);
            dst[i] = to_4b(s4 + d4 * dst_scale);
        }
    }
}

static void srcover_srgb_dst_1(const SkXfermode::PM4fState& state, uint32_t dst[],
                               const SkPM4f& src, int count, const SkAlpha aa[]) {
    Sk4f s4 = Sk4f::Load(src.fVec);
    Sk4f dst_scale = Sk4f(1 - get_alpha(s4));

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }
            Sk4f d4 = srgb_4b_to_linear_unit(dst[i]);
            Sk4f r4;
            if (a != 0xFF) {
                s4 = scale_by_coverage(s4, a);
                r4 = s4 + d4 * Sk4f(1 - get_alpha(s4));
            } else {
                r4 = s4 + d4 * dst_scale;
            }
            dst[i] = linear_unit_to_srgb_32(r4);
        }
    } else {
        while (count >= 4) {
            Sk4f d0 = srgb_4b_to_linear_unit(dst[0]);
            Sk4f d1 = srgb_4b_to_linear_unit(dst[1]);
            Sk4f d2 = srgb_4b_to_linear_unit(dst[2]);
            Sk4f d3 = srgb_4b_to_linear_unit(dst[3]);
            Sk4f_ToBytes((uint8_t*)dst,
                         linear_unit_to_srgb_255f(s4 + d0 * dst_scale),
                         linear_unit_to_srgb_255f(s4 + d1 * dst_scale),
                         linear_unit_to_srgb_255f(s4 + d2 * dst_scale),
                         linear_unit_to_srgb_255f(s4 + d3 * dst_scale));
            dst += 4;
            count -= 4;
        }
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = srgb_4b_to_linear_unit(dst[i]);
            dst[i] = to_4b(linear_unit_to_srgb_255f(s4 + d4 * dst_scale));
        }
    }
}

const XferProcPair gProcs_SrcOver[] = {
    { srcover_linear_dst_1, srcover_n<kLinear_Dst> },   // linear   alpha
    { src_1<kLinear_Dst>,   src_n<kLinear_Dst>     },   // linear   opaque [ we are src-mode ]
    { srcover_srgb_dst_1,   srcover_n<kSRGB_Dst>   },   // srgb     alpha
    { src_1<kSRGB_Dst>,     src_n<kSRGB_Dst>       },   // srgb     opaque [ we are src-mode ]
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static XferProcPair find_procs(SkXfermode::Mode mode, uint32_t flags) {
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
    return { xfer_pm4_proc_1, xfer_pm4_proc_n };
}

SkXfermode::PM4fProc1 SkXfermode::GetPM4fProc1(Mode mode, uint32_t flags) {
    return find_procs(mode, flags).fP1;
}

SkXfermode::PM4fProcN SkXfermode::GetPM4fProcN(Mode mode, uint32_t flags) {
    return find_procs(mode, flags).fPN;
}

SkXfermode::PM4fProc1 SkXfermode::getPM4fProc1(uint32_t flags) const {
    Mode mode;
    return this->asMode(&mode) ? GetPM4fProc1(mode, flags) : xfer_pm4_proc_1;
}

SkXfermode::PM4fProcN SkXfermode::getPM4fProcN(uint32_t flags) const {
    Mode mode;
    return this->asMode(&mode) ? GetPM4fProcN(mode, flags) : xfer_pm4_proc_n;
}
