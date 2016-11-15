/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPM4fPriv.h"
#include "SkUtils.h"
#include "SkXfermodePriv.h"
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

template <DstType D> uint32_t store_dst(const Sk4f& x4) {
    return (D == kSRGB_Dst) ? Sk4f_toS32(x4) : Sk4f_toL32(x4);
}

static Sk4x4f load_4_srgb(const void* vptr) {
    auto ptr = (const uint32_t*)vptr;

    Sk4x4f rgba;

    rgba.r = { sk_linear_from_srgb[(ptr[0] >>  0) & 0xff],
               sk_linear_from_srgb[(ptr[1] >>  0) & 0xff],
               sk_linear_from_srgb[(ptr[2] >>  0) & 0xff],
               sk_linear_from_srgb[(ptr[3] >>  0) & 0xff] };

    rgba.g = { sk_linear_from_srgb[(ptr[0] >>  8) & 0xff],
               sk_linear_from_srgb[(ptr[1] >>  8) & 0xff],
               sk_linear_from_srgb[(ptr[2] >>  8) & 0xff],
               sk_linear_from_srgb[(ptr[3] >>  8) & 0xff] };

    rgba.b = { sk_linear_from_srgb[(ptr[0] >> 16) & 0xff],
               sk_linear_from_srgb[(ptr[1] >> 16) & 0xff],
               sk_linear_from_srgb[(ptr[2] >> 16) & 0xff],
               sk_linear_from_srgb[(ptr[3] >> 16) & 0xff] };

    rgba.a = SkNx_cast<float>((Sk4i::Load(ptr) >> 24) & 0xff) * (1/255.0f);

    return rgba;
}

static void store_4_srgb(void* ptr, const Sk4x4f& p) {
    ( sk_linear_to_srgb(p.r) <<  0
    | sk_linear_to_srgb(p.g) <<  8
    | sk_linear_to_srgb(p.b) << 16
    | Sk4f_round(255.0f*p.a) << 24).store(ptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

template <DstType D> void general_1(SkBlendMode mode, uint32_t dst[],
                                    const SkPM4f* src, int count, const SkAlpha aa[]) {
    const SkPM4f s = rgba_to_pmcolor_order(*src);
    SkXfermodeProc4f proc = SkXfermode::GetProc4f(mode);
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

template <DstType D> void general_n(SkBlendMode mode, uint32_t dst[],
                                    const SkPM4f src[], int count, const SkAlpha aa[]) {
    SkXfermodeProc4f proc = SkXfermode::GetProc4f(mode);
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

static void clear_linear(SkBlendMode, uint32_t dst[], const SkPM4f[], int count,
                         const SkAlpha aa[]) {
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

static void clear_srgb(SkBlendMode, uint32_t dst[], const SkPM4f[], int count, const SkAlpha aa[]) {
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

template <DstType D> void src_n(SkBlendMode, uint32_t dst[], const SkPM4f src[], int count,
                                const SkAlpha aa[]) {
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

template <DstType D> void src_1(SkBlendMode, uint32_t dst[], const SkPM4f* src, int count,
                                const SkAlpha aa[]) {
    const Sk4f s4 = src->to4f_pmorder();

    if (aa) {
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

static void dst(SkBlendMode, uint32_t dst[], const SkPM4f[], int count, const SkAlpha aa[]) {}

const SkXfermode::D32Proc gProcs_Dst[] = {
    dst, dst, dst, dst, dst, dst, dst, dst,
};

///////////////////////////////////////////////////////////////////////////////////////////////////


template <DstType D> void srcover_n(SkBlendMode, uint32_t dst[], const SkPM4f src[], int count,
                                    const SkAlpha aa[]) {
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

static void srcover_linear_dst_1(SkBlendMode, uint32_t dst[], const SkPM4f* src, int count,
                                 const SkAlpha aa[]) {
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
        for (int i = 0; i < count; ++i) {
            Sk4f d4 = Sk4f_fromL32(dst[i]);
            dst[i] = Sk4f_toL32(s4 + d4 * dst_scale);
        }
    }
}

static void srcover_srgb_dst_1(SkBlendMode, uint32_t dst[], const SkPM4f* src, int count,
                               const SkAlpha aa[]) {
    Sk4f s4 = src->to4f_pmorder();
    Sk4f dst_scale = Sk4f(1 - get_alpha(s4));

    if (aa) {
        for (int i = 0; i < count; ++i) {
            unsigned a = aa[i];
            if (0 == a) {
                continue;
            }

            Sk4f d4 = Sk4f_fromS32(dst[i]);
            Sk4f r4;
            if (a != 0xFF) {
                const Sk4f s4_aa = scale_by_coverage(s4, a);
                r4 = s4_aa + d4 * Sk4f(1 - get_alpha(s4_aa));
            } else {
                r4 = s4 + d4 * dst_scale;
            }
            dst[i] = Sk4f_toS32(r4);
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
            Sk4f d4 = Sk4f_fromS32(dst[i]);
            dst[i] = Sk4f_toS32(s4 + d4 * dst_scale);
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

SkXfermode::D32Proc SkXfermode::GetD32Proc(SkBlendMode mode, uint32_t flags) {
    SkASSERT(0 == (flags & ~7));
    flags &= 7;

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
    const Sk4f s4 = src->to4f_pmorder();

    for (int i = 0; i < count; ++i) {
        uint16_t rgb = lcd[i];
        if (0 == rgb) {
            continue;
        }
        Sk4f d4 = load_dst<D>(dst[i]);
        dst[i] = store_dst<D>(lerp(s4, d4, lcd16_to_unit_4f(rgb))) | (SK_A32_MASK << SK_A32_SHIFT);
    }
}

template <DstType D>
void src_n_lcd(uint32_t dst[], const SkPM4f src[], int count, const uint16_t lcd[]) {
    for (int i = 0; i < count; ++i) {
        uint16_t rgb = lcd[i];
        if (0 == rgb) {
            continue;
        }
        Sk4f s4 = src[i].to4f_pmorder();
        Sk4f d4 = load_dst<D>(dst[i]);
        dst[i] = store_dst<D>(lerp(s4, d4, lcd16_to_unit_4f(rgb))) | (SK_A32_MASK << SK_A32_SHIFT);
    }
}

template <DstType D>
void srcover_1_lcd(uint32_t dst[], const SkPM4f* src, int count, const uint16_t lcd[]) {
    const Sk4f s4 = src->to4f_pmorder();
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
        Sk4f s4 = src[i].to4f_pmorder();
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
        srcover_n_lcd<kLinear_Dst>, src_n_lcd<kLinear_Dst>,
        srcover_1_lcd<kLinear_Dst>, src_1_lcd<kLinear_Dst>,

        srcover_n_lcd<kSRGB_Dst>,   src_n_lcd<kSRGB_Dst>,
        srcover_1_lcd<kSRGB_Dst>,   src_1_lcd<kSRGB_Dst>,
    };
    return procs[flags];
}
