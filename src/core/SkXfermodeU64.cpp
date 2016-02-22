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

static void sk_memset64(uint64_t dst[], uint64_t value, int count) {
    for (int i = 0; i < count; ++i) {
        dst[i] = value;
    }
}

struct U64ProcPair {
    SkXfermode::U64Proc1 fP1;
    SkXfermode::U64ProcN fPN;
};

enum DstType {
    kU16_Dst,
    kF16_Dst,
};

static Sk4f lerp_by_coverage(const Sk4f& src, const Sk4f& dst, uint8_t srcCoverage) {
    return dst + (src - dst) * Sk4f(srcCoverage * (1/255.0f));
}

template <DstType D> Sk4f unit_to_bias(const Sk4f& x4) {
    return (D == kU16_Dst) ? x4 * Sk4f(65535) : x4;
}

template <DstType D> Sk4f bias_to_unit(const Sk4f& x4) {
    return (D == kU16_Dst) ? x4 * Sk4f(1.0f/65535) : x4;
}

// returns value already biased by 65535
static Sk4f load_from_u16(uint64_t value) {
    return SkNx_cast<float>(Sk4h::Load(&value));
}

// takes floats already biased by 65535
static uint64_t store_to_u16(const Sk4f& x4) {
    uint64_t value;
    SkNx_cast<uint16_t>(x4 + Sk4f(0.5f)).store(&value);
    return value;
}

// Returns dst in its "natural" bias (either unit-float or 16bit int)
//
template <DstType D> Sk4f load_from_dst(uint64_t dst) {
    return (D == kU16_Dst) ? load_from_u16(dst) : SkHalfToFloat_01(dst);
}

// Assumes x4 is already in the "natural" bias (either unit-float or 16bit int)
template <DstType D> uint64_t store_to_dst(const Sk4f& x4) {
    return (D == kU16_Dst) ? store_to_u16(x4) : SkFloatToHalf_01(x4);
}

static inline Sk4f pm_to_rgba_order(const Sk4f& x) {
    if (SkPM4f::R == 0) {
        return x;   // we're already RGBA
    } else {
        // we're BGRA, so swap R and B
        return SkNx_shuffle<2, 1, 0, 3>(x);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void xfer_u64_1(const SkXfermode::U64State& state, uint64_t dst[],
                                     const SkPM4f& src, int count, const SkAlpha aa[]) {
    SkXfermodeProc4f proc = state.fXfer->getProc4f();
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = bias_to_unit<D>(load_from_dst<D>(dst[i]));
            d4.store(d.fVec);
            Sk4f r4 = unit_to_bias<D>(Sk4f::Load(proc(src, d).fVec));
            dst[i] = store_to_dst<D>(lerp_by_coverage(r4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            bias_to_unit<D>(load_from_dst<D>(dst[i])).store(d.fVec);
            Sk4f r4 = unit_to_bias<D>(Sk4f::Load(proc(src, d).fVec));
            dst[i] = store_to_dst<D>(r4);
        }
    }
}

template <DstType D> void xfer_u64_n(const SkXfermode::U64State& state, uint64_t dst[],
                                     const SkPM4f src[], int count, const SkAlpha aa[]) {
    SkXfermodeProc4f proc = state.fXfer->getProc4f();
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = bias_to_unit<D>(load_from_dst<D>(dst[i]));
            d4.store(d.fVec);
            Sk4f r4 = unit_to_bias<D>(Sk4f::Load(proc(src[i], d).fVec));
            dst[i] = store_to_dst<D>(lerp_by_coverage(r4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            bias_to_unit<D>(load_from_dst<D>(dst[i])).store(d.fVec);
            Sk4f r4 = unit_to_bias<D>(Sk4f::Load(proc(src[i], d).fVec));
            dst[i] = store_to_dst<D>(r4);
        }
    }
}

const U64ProcPair gU64Procs_General[] = {
    { xfer_u64_1<kU16_Dst>, xfer_u64_n<kU16_Dst> },   // U16     alpha
    { xfer_u64_1<kU16_Dst>, xfer_u64_n<kU16_Dst> },   // U16     opaque
    { xfer_u64_1<kF16_Dst>, xfer_u64_n<kF16_Dst> },   // F16     alpha
    { xfer_u64_1<kF16_Dst>, xfer_u64_n<kF16_Dst> },   // F16     opaque
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void src_1(const SkXfermode::U64State& state, uint64_t dst[],
                                const SkPM4f& src, int count, const SkAlpha aa[]) {
    const Sk4f s4 = pm_to_rgba_order(unit_to_bias<D>(Sk4f::Load(src.fVec)));
    if (aa) {
        for (int i = 0; i < count; ++i) {
            const Sk4f d4 = load_from_dst<D>(dst[i]);
            dst[i] = store_to_dst<D>(lerp_by_coverage(s4, d4, aa[i]));
        }
    } else {
        sk_memset64(dst, store_to_dst<D>(s4), count);
    }
}

template <DstType D> void src_n(const SkXfermode::U64State& state, uint64_t dst[],
                                const SkPM4f src[], int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            const Sk4f s4 = pm_to_rgba_order(unit_to_bias<D>(Sk4f::Load(src[i].fVec)));
            const Sk4f d4 = load_from_dst<D>(dst[i]);
            dst[i] = store_to_dst<D>(lerp_by_coverage(s4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            const Sk4f s4 = pm_to_rgba_order(unit_to_bias<D>(Sk4f::Load(src[i].fVec)));
            dst[i] = store_to_dst<D>(s4);
        }
    }
}

const U64ProcPair gU64Procs_Src[] = {
    { src_1<kU16_Dst>, src_n<kU16_Dst>  },   // U16     alpha
    { src_1<kU16_Dst>, src_n<kU16_Dst>  },   // U16     opaque
    { src_1<kF16_Dst>, src_n<kF16_Dst>  },   // F16     alpha
    { src_1<kF16_Dst>, src_n<kF16_Dst>  },   // F16     opaque
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void srcover_1(const SkXfermode::U64State& state, uint64_t dst[],
                                    const SkPM4f& src, int count, const SkAlpha aa[]) {
    const Sk4f s4 = pm_to_rgba_order(Sk4f::Load(src.fVec));
    const Sk4f dst_scale = Sk4f(1 - get_alpha(s4));
    const Sk4f s4bias = unit_to_bias<D>(s4);
    for (int i = 0; i < count; ++i) {
        const Sk4f d4bias = load_from_dst<D>(dst[i]);
        const Sk4f r4bias = s4bias + d4bias * dst_scale;
        if (aa) {
            dst[i] = store_to_dst<D>(lerp_by_coverage(r4bias, d4bias, aa[i]));
        } else {
            dst[i] = store_to_dst<D>(r4bias);
        }
    }
}

template <DstType D> void srcover_n(const SkXfermode::U64State& state, uint64_t dst[],
                                    const SkPM4f src[], int count, const SkAlpha aa[]) {
    for (int i = 0; i < count; ++i) {
        const Sk4f s4 = pm_to_rgba_order(Sk4f::Load(src[i].fVec));
        const Sk4f dst_scale = Sk4f(1 - get_alpha(s4));
        const Sk4f s4bias = unit_to_bias<D>(s4);
        const Sk4f d4bias = load_from_dst<D>(dst[i]);
        const Sk4f r4bias = s4bias + d4bias * dst_scale;
        if (aa) {
            dst[i] = store_to_dst<D>(lerp_by_coverage(r4bias, d4bias, aa[i]));
        } else {
            dst[i] = store_to_dst<D>(r4bias);
        }
    }
}

const U64ProcPair gU64Procs_SrcOver[] = {
    { srcover_1<kU16_Dst>,  srcover_n<kU16_Dst> },   // U16     alpha
    { src_1<kU16_Dst>,      src_n<kU16_Dst>     },   // U16     opaque
    { srcover_1<kF16_Dst>,  srcover_n<kF16_Dst> },   // F16     alpha
    { src_1<kF16_Dst>,      src_n<kF16_Dst>     },   // F16     opaque
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static U64ProcPair find_procs(SkXfermode::Mode mode, uint32_t flags) {
    SkASSERT(0 == (flags & ~3));
    flags &= 3;

    switch (mode) {
        case SkXfermode::kSrc_Mode:     return gU64Procs_Src[flags];
        case SkXfermode::kSrcOver_Mode: return gU64Procs_SrcOver[flags];
        default:
            break;
    }
    return gU64Procs_General[flags];
}

SkXfermode::U64Proc1 SkXfermode::GetU64Proc1(Mode mode, uint32_t flags) {
    return find_procs(mode, flags).fP1;
}

SkXfermode::U64ProcN SkXfermode::GetU64ProcN(Mode mode, uint32_t flags) {
    return find_procs(mode, flags).fPN;
}
