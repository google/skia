/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPM4fPriv.h"
#include "SkUtils.h"
#include "SkXfermode.h"
#include "Sk4x4f.h"

static SkPM4f rgba_to_pmcolor_order(const SkPM4f& x) {
#ifdef SK_PMCOLOR_IS_BGRA
    return {{ x.fVec[2], x.fVec[1], x.fVec[0], x.fVec[3] }};
#else
    return x;
#endif
}

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

static Sk4f linear_unit_to_srgb_255f(const Sk4f& l4) {
    return linear_to_srgb(l4) * Sk4f(255) + Sk4f(0.5f);
}

// Load 4 interlaced 8888 sRGB pixels as an Sk4x4f, transposed and converted to float.
static Sk4x4f load_4_srgb(const void* ptr) {
    auto p = Sk4x4f::Transpose((const uint8_t*)ptr);

    // Scale to [0,1].
    p.r *= 1/255.0f;
    p.g *= 1/255.0f;
    p.b *= 1/255.0f;
    p.a *= 1/255.0f;

    // Apply approximate sRGB gamma correction to convert to linear (as if gamma were 2).
    p.r *= p.r;
    p.g *= p.g;
    p.b *= p.b;

    return p;
}

// Store an Sk4x4f back to 4 interlaced 8888 sRGB pixels.
static void store_4_srgb(void* ptr, const Sk4x4f& p) {
    // Convert back to sRGB and [0,255], again approximating sRGB as gamma == 2.
    auto r = p.r.rsqrt().invert() * 255.0f + 0.5f,
         g = p.g.rsqrt().invert() * 255.0f + 0.5f,
         b = p.b.rsqrt().invert() * 255.0f + 0.5f,
         a = p.a                  * 255.0f + 0.5f;
    Sk4x4f{r,g,b,a}.transpose((uint8_t*)ptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void general_1(const SkXfermode* xfer, uint32_t dst[],
                                    const SkPM4f* src, int count, const SkAlpha aa[]) {
    const SkPM4f s = rgba_to_pmcolor_order(*src);
    SkXfermodeProc4f proc = xfer->getProc4f();
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = load_dst<D>(dst[i]);
            d4.store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(s, d).fVec);
            dst[i] = store_dst<D>(lerp(r4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            load_dst<D>(dst[i]).store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(s, d).fVec);
            dst[i] = store_dst<D>(r4);
        }
    }
}

template <DstType D> void general_n(const SkXfermode* xfer, uint32_t dst[],
                                    const SkPM4f src[], int count, const SkAlpha aa[]) {
    SkXfermodeProc4f proc = xfer->getProc4f();
    SkPM4f d;
    if (aa) {
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = load_dst<D>(dst[i]);
            d4.store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(rgba_to_pmcolor_order(src[i]), d).fVec);
            dst[i] = store_dst<D>(lerp(r4, d4, aa[i]));
        }
    } else {
        for (int i = 0; i < count; ++i) {
            load_dst<D>(dst[i]).store(d.fVec);
            Sk4f r4 = Sk4f::Load(proc(rgba_to_pmcolor_order(src[i]), d).fVec);
            dst[i] = store_dst<D>(r4);
        }
    }
}

const SkXfermode::D32Proc gProcs_General[] = {
    general_n<kLinear_Dst>, general_n<kLinear_Dst>,
    general_1<kLinear_Dst>, general_1<kLinear_Dst>,
    general_n<kSRGB_Dst>,   general_n<kSRGB_Dst>,
    general_1<kSRGB_Dst>,   general_1<kSRGB_Dst>,
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void clear_linear(const SkXfermode*, uint32_t dst[], const SkPM4f[],
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
        sk_memset32(dst, 0, count);
    }
}

static void clear_srgb(const SkXfermode*, uint32_t dst[], const SkPM4f[],
                       int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            if (aa[i]) {
                Sk4f d = Sk4f_fromS32(dst[i]) * Sk4f((255 - aa[i]) * (1/255.0f));
                dst[i] = Sk4f_toS32(d);
            }
        }
    } else {
        sk_memset32(dst, 0, count);
    }
}

const SkXfermode::D32Proc gProcs_Clear[] = {
    clear_linear,   clear_linear,
    clear_linear,   clear_linear,
    clear_srgb,     clear_srgb,
    clear_srgb,     clear_srgb,
};

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void src_n(const SkXfermode*, uint32_t dst[],
                                const SkPM4f src[], int count, const SkAlpha aa[]) {
    for (int i = 0; i < count; ++i) {
        unsigned a = 0xFF;
        if (aa) {
            a = aa[i];
            if (0 == a) {
                continue;
            }
        }
        Sk4f r4 = src[i].to4f_pmorder();
        if (a != 0xFF) {
            Sk4f d4 = load_dst<D>(dst[i]);
            r4 = lerp(r4, d4, a);
        }
        dst[i] = store_dst<D>(r4);
    }
}

static Sk4f lerp(const Sk4f& src, const Sk4f& dst, const Sk4f& src_scale) {
    return dst + (src - dst) * src_scale;
}

template <DstType D> void src_1(const SkXfermode*, uint32_t dst[],
                                const SkPM4f* src, int count, const SkAlpha aa[]) {
    const Sk4f s4 = src->to4f_pmorder();

    if (aa) {
        if (D == kLinear_Dst) {
            // operate in bias-255 space for src and dst
            const Sk4f& s4_255 = s4 * Sk4f(255);
            while (count >= 4) {
                Sk4f aa4 = SkNx_cast<float>(Sk4b::Load(aa)) * Sk4f(1/255.f);
                Sk4f r0 = lerp(s4_255, to_4f(dst[0]), Sk4f(aa4[0])) + Sk4f(0.5f);
                Sk4f r1 = lerp(s4_255, to_4f(dst[1]), Sk4f(aa4[1])) + Sk4f(0.5f);
                Sk4f r2 = lerp(s4_255, to_4f(dst[2]), Sk4f(aa4[2])) + Sk4f(0.5f);
                Sk4f r3 = lerp(s4_255, to_4f(dst[3]), Sk4f(aa4[3])) + Sk4f(0.5f);
                Sk4f_ToBytes((uint8_t*)dst, r0, r1, r2, r3);

                dst += 4;
                aa += 4;
                count -= 4;
            }
        } else {    // kSRGB
            SkPMColor srcColor = store_dst<D>(s4);
            while (count-- > 0) {
                SkAlpha cover = *aa++;
                switch (cover) {
                    case 0xFF: {
                        *dst++ = srcColor;
                        break;
                    }
                    case 0x00: {
                        dst++;
                        break;
                    }
                    default: {
                        Sk4f d4 = load_dst<D>(*dst);
                        *dst++ = store_dst<D>(lerp(s4, d4, cover));
                    }
                }
            }
        }          // kSRGB
    } else {
        sk_memset32(dst, store_dst<D>(s4), count);
    }
}

const SkXfermode::D32Proc gProcs_Src[] = {
    src_n<kLinear_Dst>, src_n<kLinear_Dst>,
    src_1<kLinear_Dst>, src_1<kLinear_Dst>,
    src_n<kSRGB_Dst>,   src_n<kSRGB_Dst>,
    src_1<kSRGB_Dst>,   src_1<kSRGB_Dst>,
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static void dst(const SkXfermode*, uint32_t dst[], const SkPM4f[], int count, const SkAlpha aa[]) {}

const SkXfermode::D32Proc gProcs_Dst[] = {
    dst, dst, dst, dst, dst, dst, dst, dst,
};

///////////////////////////////////////////////////////////////////////////////////////////////////


template <DstType D> void srcover_n(const SkXfermode*, uint32_t dst[],
                                    const SkPM4f src[], int count, const SkAlpha aa[]) {
    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }
            Sk4f s4 = src[i].to4f_pmorder();
            Sk4f d4 = load_dst<D>(dst[i]);
            if (a != 0xFF) {
                s4 = scale_by_coverage(s4, a);
            }
            Sk4f r4 = s4 + d4 * Sk4f(1 - get_alpha(s4));
            dst[i] = store_dst<D>(r4);
        }
    } else {
        while (count >= 4 && D == kSRGB_Dst) {
            auto d = load_4_srgb(dst);

            auto s = Sk4x4f::Transpose(src->fVec);
        #if defined(SK_PMCOLOR_IS_BGRA)
            SkTSwap(s.r, s.b);
        #endif

            auto invSA = 1.0f - s.a;
            auto r = s.r + d.r * invSA,
                 g = s.g + d.g * invSA,
                 b = s.b + d.b * invSA,
                 a = s.a + d.a * invSA;

            store_4_srgb(dst, Sk4x4f{r,g,b,a});
            count -= 4;
            dst += 4;
            src += 4;
        }
        for (int i = 0; i < count; ++i) {
            Sk4f s4 = src[i].to4f_pmorder();
            Sk4f d4 = load_dst<D>(dst[i]);
            Sk4f r4 = s4 + d4 * Sk4f(1 - get_alpha(s4));
            dst[i] = store_dst<D>(r4);
        }
    }
}

static void srcover_linear_dst_1(const SkXfermode*, uint32_t dst[],
                                 const SkPM4f* src, int count, const SkAlpha aa[]) {
    const Sk4f s4 = src->to4f_pmorder();
    const Sk4f dst_scale = Sk4f(1 - get_alpha(s4));

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }
            Sk4f d4 = Sk4f_fromL32(dst[i]);
            Sk4f r4;
            if (a != 0xFF) {
                Sk4f s4_aa = scale_by_coverage(s4, a);
                r4 = s4_aa + d4 * Sk4f(1 - get_alpha(s4_aa));
            } else {
                r4 = s4 + d4 * dst_scale;
            }
            dst[i] = Sk4f_toL32(r4);
        }
    } else {
        const Sk4f s4_255 = s4 * Sk4f(255) + Sk4f(0.5f);   // +0.5 to pre-bias for rounding
        while (count >= 4) {
            Sk4f d0 = to_4f(dst[0]);
            Sk4f d1 = to_4f(dst[1]);
            Sk4f d2 = to_4f(dst[2]);
            Sk4f d3 = to_4f(dst[3]);
            Sk4f_ToBytes((uint8_t*)dst,
                         s4_255 + d0 * dst_scale,
                         s4_255 + d1 * dst_scale,
                         s4_255 + d2 * dst_scale,
                         s4_255 + d3 * dst_scale);
            dst += 4;
            count -= 4;
        }
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = to_4f(dst[i]);
            dst[i] = to_4b(s4_255 + d4 * dst_scale);
        }
    }
}

static void srcover_srgb_dst_1(const SkXfermode*, uint32_t dst[],
                               const SkPM4f* src, int count, const SkAlpha aa[]) {
    Sk4f s4 = src->to4f_pmorder();
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
                const Sk4f s4_aa = scale_by_coverage(s4, a);
                r4 = s4_aa + d4 * Sk4f(1 - get_alpha(s4_aa));
            } else {
                r4 = s4 + d4 * dst_scale;
            }
            dst[i] = to_4b(linear_unit_to_srgb_255f(r4));
        }
    } else {
        while (count >= 4) {
            auto d = load_4_srgb(dst);

            auto s = Sk4x4f{{ src->r() }, { src->g() }, { src->b() }, { src->a() }};
        #if defined(SK_PMCOLOR_IS_BGRA)
            SkTSwap(s.r, s.b);
        #endif

            auto invSA = 1.0f - s.a;
            auto r = s.r + d.r * invSA,
                 g = s.g + d.g * invSA,
                 b = s.b + d.b * invSA,
                 a = s.a + d.a * invSA;

            store_4_srgb(dst, Sk4x4f{r,g,b,a});
            count -= 4;
            dst += 4;
        }
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = srgb_4b_to_linear_unit(dst[i]);
            dst[i] = to_4b(linear_unit_to_srgb_255f(s4 + d4 * dst_scale));
        }
    }
}

const SkXfermode::D32Proc gProcs_SrcOver[] = {
    srcover_n<kLinear_Dst>, src_n<kLinear_Dst>,
    srcover_linear_dst_1,   src_1<kLinear_Dst>,

    srcover_n<kSRGB_Dst>,   src_n<kSRGB_Dst>,
    srcover_srgb_dst_1,     src_1<kSRGB_Dst>,
};

///////////////////////////////////////////////////////////////////////////////////////////////////

static SkXfermode::D32Proc find_proc(SkXfermode::Mode mode, uint32_t flags) {
    SkASSERT(0 == (flags & ~7));
    flags &= 7;

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

SkXfermode::D32Proc SkXfermode::onGetD32Proc(uint32_t flags) const {
    SkASSERT(0 == (flags & ~7));
    flags &= 7;

    Mode mode;
    return this->asMode(&mode) ? find_proc(mode, flags) : gProcs_General[flags];
}

SkXfermode::D32Proc SkXfermode::GetD32Proc(SkXfermode* xfer, uint32_t flags) {
    return xfer ? xfer->onGetD32Proc(flags) : find_proc(SkXfermode::kSrcOver_Mode, flags);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkColorPriv.h"

static Sk4f lcd16_to_unit_4f(uint16_t rgb) {
#ifdef SK_PMCOLOR_IS_RGBA
    Sk4i rgbi = Sk4i(SkGetPackedR16(rgb), SkGetPackedG16(rgb), SkGetPackedB16(rgb), 0);
#else
    Sk4i rgbi = Sk4i(SkGetPackedB16(rgb), SkGetPackedG16(rgb), SkGetPackedR16(rgb), 0);
#endif
    return SkNx_cast<float>(rgbi) * Sk4f(1.0f/31, 1.0f/63, 1.0f/31, 0);
}

template <DstType D>
void src_1_lcd(uint32_t dst[], const SkPM4f* src, int count, const uint16_t lcd[]) {
    const Sk4f s4 = Sk4f::Load(src->fVec);

    if (D == kLinear_Dst) {
        // operate in bias-255 space for src and dst
        const Sk4f s4bias = s4 * Sk4f(255);
        for (int i = 0; i < count; ++i) {
            uint16_t rgb = lcd[i];
            if (0 == rgb) {
                continue;
            }
            Sk4f d4bias = to_4f(dst[i]);
            dst[i] = to_4b(lerp(s4bias, d4bias, lcd16_to_unit_4f(rgb))) | (SK_A32_MASK << SK_A32_SHIFT);
        }
    } else {    // kSRGB
        for (int i = 0; i < count; ++i) {
            uint16_t rgb = lcd[i];
            if (0 == rgb) {
                continue;
            }
            Sk4f d4 = load_dst<D>(dst[i]);
            dst[i] = store_dst<D>(lerp(s4, d4, lcd16_to_unit_4f(rgb))) | (SK_A32_MASK << SK_A32_SHIFT);
        }
    }
}

template <DstType D>
void src_n_lcd(uint32_t dst[], const SkPM4f src[], int count, const uint16_t lcd[]) {
    for (int i = 0; i < count; ++i) {
        uint16_t rgb = lcd[i];
        if (0 == rgb) {
            continue;
        }
        Sk4f s4 = Sk4f::Load(src[i].fVec);
        Sk4f d4 = load_dst<D>(dst[i]);
        dst[i] = store_dst<D>(lerp(s4, d4, lcd16_to_unit_4f(rgb))) | (SK_A32_MASK << SK_A32_SHIFT);
    }
}

template <DstType D>
void srcover_1_lcd(uint32_t dst[], const SkPM4f* src, int count, const uint16_t lcd[]) {
    const Sk4f s4 = Sk4f::Load(src->fVec);
    Sk4f dst_scale = Sk4f(1 - get_alpha(s4));

    for (int i = 0; i < count; ++i) {
        uint16_t rgb = lcd[i];
        if (0 == rgb) {
            continue;
        }
        Sk4f d4 = load_dst<D>(dst[i]);
        Sk4f r4 = s4 + d4 * dst_scale;
        r4 = lerp(r4, d4, lcd16_to_unit_4f(rgb));
        dst[i] = store_dst<D>(r4) | (SK_A32_MASK << SK_A32_SHIFT);
    }
}

template <DstType D>
void srcover_n_lcd(uint32_t dst[], const SkPM4f src[], int count, const uint16_t lcd[]) {
    for (int i = 0; i < count; ++i) {
        uint16_t rgb = lcd[i];
        if (0 == rgb) {
            continue;
        }
        Sk4f s4 = Sk4f::Load(src[i].fVec);
        Sk4f dst_scale = Sk4f(1 - get_alpha(s4));
        Sk4f d4 = load_dst<D>(dst[i]);
        Sk4f r4 = s4 + d4 * dst_scale;
        r4 = lerp(r4, d4, lcd16_to_unit_4f(rgb));
        dst[i] = store_dst<D>(r4) | (SK_A32_MASK << SK_A32_SHIFT);
    }
}

SkXfermode::LCD32Proc SkXfermode::GetLCD32Proc(uint32_t flags) {
    SkASSERT((flags & ~7) == 0);
    flags &= 7;

    const LCD32Proc procs[] = {
        srcover_n_lcd<kSRGB_Dst>,   src_n_lcd<kSRGB_Dst>,
        srcover_1_lcd<kSRGB_Dst>,   src_1_lcd<kSRGB_Dst>,

        srcover_n_lcd<kLinear_Dst>, src_n_lcd<kLinear_Dst>,
        srcover_1_lcd<kLinear_Dst>, src_1_lcd<kLinear_Dst>,
    };
    return procs[flags];
}
