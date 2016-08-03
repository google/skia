/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpaceXformOpts_DEFINED
#define SkColorSpaceXformOpts_DEFINED

#include "SkNx.h"
#include "SkColorPriv.h"
#include "SkHalf.h"
#include "SkSRGB.h"
#include "SkTemplates.h"

enum SwapRB {
    kNo_SwapRB,
    kYes_SwapRB,
};

static inline void load_matrix(const float matrix[16],
                               Sk4f& rXgXbX, Sk4f& rYgYbY, Sk4f& rZgZbZ, Sk4f& rTgTbT) {
    rXgXbX = Sk4f::Load(matrix +  0);
    rYgYbY = Sk4f::Load(matrix +  4);
    rZgZbZ = Sk4f::Load(matrix +  8);
    rTgTbT = Sk4f::Load(matrix + 12);
}

static inline void load_rgb_from_tables(const uint32_t* src,
                                        Sk4f& r, Sk4f& g, Sk4f& b, Sk4f&,
                                        const float* const srcTables[3]) {
    r = { srcTables[0][(src[0] >>  0) & 0xFF],
          srcTables[0][(src[1] >>  0) & 0xFF],
          srcTables[0][(src[2] >>  0) & 0xFF],
          srcTables[0][(src[3] >>  0) & 0xFF], };
    g = { srcTables[1][(src[0] >>  8) & 0xFF],
          srcTables[1][(src[1] >>  8) & 0xFF],
          srcTables[1][(src[2] >>  8) & 0xFF],
          srcTables[1][(src[3] >>  8) & 0xFF], };
    b = { srcTables[2][(src[0] >> 16) & 0xFF],
          srcTables[2][(src[1] >> 16) & 0xFF],
          srcTables[2][(src[2] >> 16) & 0xFF],
          srcTables[2][(src[3] >> 16) & 0xFF], };
}

static inline void load_rgba_from_tables(const uint32_t* src,
                                         Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                         const float* const srcTables[3]) {
    r = { srcTables[0][(src[0] >>  0) & 0xFF],
          srcTables[0][(src[1] >>  0) & 0xFF],
          srcTables[0][(src[2] >>  0) & 0xFF],
          srcTables[0][(src[3] >>  0) & 0xFF], };
    g = { srcTables[1][(src[0] >>  8) & 0xFF],
          srcTables[1][(src[1] >>  8) & 0xFF],
          srcTables[1][(src[2] >>  8) & 0xFF],
          srcTables[1][(src[3] >>  8) & 0xFF], };
    b = { srcTables[2][(src[0] >> 16) & 0xFF],
          srcTables[2][(src[1] >> 16) & 0xFF],
          srcTables[2][(src[2] >> 16) & 0xFF],
          srcTables[2][(src[3] >> 16) & 0xFF], };
    a = (1.0f / 255.0f) * SkNx_cast<float>(Sk4u::Load(src) >> 24);
}

static inline void load_rgb_from_tables_1(const uint32_t* src,
                                          Sk4f& r, Sk4f& g, Sk4f& b, Sk4f&,
                                          const float* const srcTables[3]) {
    // Splat r,g,b across a register each.
    r = Sk4f(srcTables[0][(*src >>  0) & 0xFF]);
    g = Sk4f(srcTables[1][(*src >>  8) & 0xFF]);
    b = Sk4f(srcTables[2][(*src >> 16) & 0xFF]);
}

static inline void load_rgba_from_tables_1(const uint32_t* src,
                                           Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a,
                                           const float* const srcTables[3]) {
    // Splat r,g,b across a register each.
    r = Sk4f(srcTables[0][(*src >>  0) & 0xFF]);
    g = Sk4f(srcTables[1][(*src >>  8) & 0xFF]);
    b = Sk4f(srcTables[2][(*src >> 16) & 0xFF]);
    a = (1.0f / 255.0f) * Sk4f(*src >> 24);
}

static inline void transform_gamut(const Sk4f& r, const Sk4f& g, const Sk4f& b, const Sk4f& a,
                                   const Sk4f& rXgXbX, const Sk4f& rYgYbY, const Sk4f& rZgZbZ,
                                   Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da) {
    dr = rXgXbX[0]*r + rYgYbY[0]*g + rZgZbZ[0]*b;
    dg = rXgXbX[1]*r + rYgYbY[1]*g + rZgZbZ[1]*b;
    db = rXgXbX[2]*r + rYgYbY[2]*g + rZgZbZ[2]*b;
    da = a;
}

static inline void transform_gamut_1(const Sk4f& r, const Sk4f& g, const Sk4f& b,
                                     const Sk4f& rXgXbX, const Sk4f& rYgYbY, const Sk4f& rZgZbZ,
                                     Sk4f& rgba) {
    rgba = rXgXbX*r + rYgYbY*g + rZgZbZ*b;
}

static inline void translate_gamut(const Sk4f& rTgTbT, Sk4f& dr, Sk4f& dg, Sk4f& db) {
    dr = dr + rTgTbT[0];
    dg = dg + rTgTbT[1];
    db = db + rTgTbT[2];
}

static inline void translate_gamut_1(const Sk4f& rTgTbT, Sk4f& rgba) {
    rgba = rgba + rTgTbT;
}

static inline void premultiply(Sk4f& dr, Sk4f& dg, Sk4f& db, const Sk4f& da) {
    dr = da * dr;
    dg = da * dg;
    db = da * db;
}

static inline void premultiply_1(const Sk4f& a, Sk4f& rgba) {
    rgba = a * rgba;
}

static inline void store_srgb(void* dst, const uint32_t* src,
                              Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                              const uint8_t* const[3], SwapRB kSwapRB) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = sk_linear_to_srgb_needs_trunc(dr);
    dg = sk_linear_to_srgb_needs_trunc(dg);
    db = sk_linear_to_srgb_needs_trunc(db);

    dr = sk_clamp_0_255(dr);
    dg = sk_clamp_0_255(dg);
    db = sk_clamp_0_255(db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    Sk4i rgba = (SkNx_cast<int>(dr) << kRShift)
              | (SkNx_cast<int>(dg) << kGShift)
              | (SkNx_cast<int>(db) << kBShift)
              | (da                           );
    rgba.store(dst);
}

static inline void store_srgb_1(void* dst, const uint32_t* src,
                                Sk4f& rgba, const Sk4f&,
                                const uint8_t* const[3], SwapRB kSwapRB) {
    rgba = sk_clamp_0_255(sk_linear_to_srgb_needs_trunc(rgba));

    uint32_t tmp;
    SkNx_cast<uint8_t>(SkNx_cast<int32_t>(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kYes_SwapRB == kSwapRB) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

static inline Sk4f linear_to_2dot2(const Sk4f& x) {
    // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
    auto x2  = x.rsqrt(),                            // x^(-1/2)
         x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
         x64 = x32.rsqrt();                          // x^(+1/64)

    // 29 = 32 - 2 - 1
    return 255.0f * x2.invert() * x32 * x64.invert();
}

static inline void store_2dot2(void* dst, const uint32_t* src,
                               Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                               const uint8_t* const[3], SwapRB kSwapRB) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = linear_to_2dot2(dr);
    dg = linear_to_2dot2(dg);
    db = linear_to_2dot2(db);

    dr = sk_clamp_0_255(dr);
    dg = sk_clamp_0_255(dg);
    db = sk_clamp_0_255(db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    Sk4i rgba = (Sk4f_round(dr) << kRShift)
              | (Sk4f_round(dg) << kGShift)
              | (Sk4f_round(db) << kBShift)
              | (da                       );
    rgba.store(dst);
}

static inline void store_2dot2_1(void* dst, const uint32_t* src,
                                 Sk4f& rgba, const Sk4f&,
                                 const uint8_t* const[3], SwapRB kSwapRB) {
    rgba = sk_clamp_0_255(linear_to_2dot2(rgba));

    uint32_t tmp;
    SkNx_cast<uint8_t>(Sk4f_round(rgba)).store(&tmp);
    tmp = (*src & 0xFF000000) | (tmp & 0x00FFFFFF);
    if (kYes_SwapRB == kSwapRB) {
        tmp = SkSwizzle_RB(tmp);
    }

    *(uint32_t*)dst = tmp;
}

static inline void store_f16(void* dst, const uint32_t* src,
                             Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da,
                             const uint8_t* const[3], SwapRB) {
    Sk4h_store4(dst, SkFloatToHalf_finite(dr),
                     SkFloatToHalf_finite(dg),
                     SkFloatToHalf_finite(db),
                     SkFloatToHalf_finite(da));
}

static inline void store_f16_1(void* dst, const uint32_t* src,
                               Sk4f& rgba, const Sk4f& a,
                               const uint8_t* const[3], SwapRB kSwapRB) {
    rgba = Sk4f(rgba[0], rgba[1], rgba[2], a[3]);
    SkFloatToHalf_finite(rgba).store((uint64_t*) dst);
}

static inline void store_f16_opaque(void* dst, const uint32_t* src,
                                    Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da,
                                    const uint8_t* const[3], SwapRB) {
    Sk4h_store4(dst, SkFloatToHalf_finite(dr),
                     SkFloatToHalf_finite(dg),
                     SkFloatToHalf_finite(db),
                     SK_Half1);
}

static inline void store_f16_1_opaque(void* dst, const uint32_t* src,
                                      Sk4f& rgba, const Sk4f& a,
                                      const uint8_t* const[3], SwapRB kSwapRB) {
    uint64_t tmp;
    SkFloatToHalf_finite(rgba).store(&tmp);
    tmp |= static_cast<uint64_t>(SK_Half1) << 48;
    *((uint64_t*) dst) = tmp;
}

static inline void store_generic(void* dst, const uint32_t* src,
                                 Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f&,
                                 const uint8_t* const dstTables[3], SwapRB kSwapRB) {
    int kRShift = 0;
    int kGShift = 8;
    int kBShift = 16;
    if (kYes_SwapRB == kSwapRB) {
        kBShift = 0;
        kRShift = 16;
    }

    dr = Sk4f::Min(Sk4f::Max(1023.0f * dr, 0.0f), 1023.0f);
    dg = Sk4f::Min(Sk4f::Max(1023.0f * dg, 0.0f), 1023.0f);
    db = Sk4f::Min(Sk4f::Max(1023.0f * db, 0.0f), 1023.0f);

    Sk4i ir = Sk4f_round(dr);
    Sk4i ig = Sk4f_round(dg);
    Sk4i ib = Sk4f_round(db);

    Sk4i da = Sk4i::Load(src) & 0xFF000000;

    uint32_t* dst32 = (uint32_t*) dst;
    dst32[0] = dstTables[0][ir[0]] << kRShift
             | dstTables[1][ig[0]] << kGShift
             | dstTables[2][ib[0]] << kBShift
             | da[0];
    dst32[1] = dstTables[0][ir[1]] << kRShift
             | dstTables[1][ig[1]] << kGShift
             | dstTables[2][ib[1]] << kBShift
             | da[1];
    dst32[2] = dstTables[0][ir[2]] << kRShift
             | dstTables[1][ig[2]] << kGShift
             | dstTables[2][ib[2]] << kBShift
             | da[2];
    dst32[3] = dstTables[0][ir[3]] << kRShift
             | dstTables[1][ig[3]] << kGShift
             | dstTables[2][ib[3]] << kBShift
             | da[3];
}

static inline void store_generic_1(void* dst, const uint32_t* src,
                                   Sk4f& rgba, const Sk4f&,
                                   const uint8_t* const dstTables[3], SwapRB kSwapRB) {
    rgba = Sk4f::Min(Sk4f::Max(1023.0f * rgba, 0.0f), 1023.0f);

    Sk4i indices = Sk4f_round(rgba);

    *((uint32_t*) dst) = dstTables[0][indices[0]] <<  0
                       | dstTables[1][indices[1]] <<  8
                       | dstTables[2][indices[2]] << 16
                       | (*src & 0xFF000000);
}

template <SkColorSpace::GammaNamed kDstGamma, SkAlphaType kAlphaType, SwapRB kSwapRB>
static void color_xform_RGBA(void* dst, const uint32_t* src, int len,
                             const float* const srcTables[3], const float matrix[16],
                             const uint8_t* const dstTables[3]) {
    decltype(store_srgb            )* store;
    decltype(store_srgb_1          )* store_1;
    decltype(load_rgb_from_tables  )* load;
    decltype(load_rgb_from_tables_1)* load_1;
    size_t sizeOfDstPixel;
    switch (kDstGamma) {
        case SkColorSpace::kSRGB_GammaNamed:
            load    = (kPremul_SkAlphaType == kAlphaType) ? load_rgba_from_tables :
                                                            load_rgb_from_tables;
            load_1  = (kPremul_SkAlphaType == kAlphaType) ? load_rgba_from_tables_1 :
                                                            load_rgb_from_tables_1;
            store   = store_srgb;
            store_1 = store_srgb_1;
            sizeOfDstPixel = 4;
            break;
        case SkColorSpace::k2Dot2Curve_GammaNamed:
            load    = (kPremul_SkAlphaType == kAlphaType) ? load_rgba_from_tables :
                                                            load_rgb_from_tables;
            load_1  = (kPremul_SkAlphaType == kAlphaType) ? load_rgba_from_tables_1 :
                                                            load_rgb_from_tables_1;
            store   = store_2dot2;
            store_1 = store_2dot2_1;
            sizeOfDstPixel = 4;
            break;
        case SkColorSpace::kLinear_GammaNamed:
            load    = load_rgba_from_tables;
            load_1  = load_rgba_from_tables_1;
            store   = (kOpaque_SkAlphaType == kAlphaType) ? store_f16_opaque :
                                                            store_f16;
            store_1 = (kOpaque_SkAlphaType == kAlphaType) ? store_f16_1_opaque :
                                                            store_f16_1;
            sizeOfDstPixel = 8;
            break;
        case SkColorSpace::kNonStandard_GammaNamed:
            load    = (kPremul_SkAlphaType == kAlphaType) ? load_rgba_from_tables :
                                                            load_rgb_from_tables;
            load_1  = (kPremul_SkAlphaType == kAlphaType) ? load_rgba_from_tables_1 :
                                                            load_rgb_from_tables_1;
            store   = store_generic;
            store_1 = store_generic_1;
            sizeOfDstPixel = 4;
            break;
    }

    Sk4f rXgXbX, rYgYbY, rZgZbZ, rTgTbT;
    load_matrix(matrix, rXgXbX, rYgYbY, rZgZbZ, rTgTbT);

    if (len >= 4) {
        // Naively this would be a loop of load-transform-store, but we found it faster to
        // move the N+1th load ahead of the Nth store.  We don't bother doing this for N<4.
        Sk4f r, g, b, a;
        load(src, r, g, b, a, srcTables);
        src += 4;
        len -= 4;

        Sk4f dr, dg, db, da;
        while (len >= 4) {
            transform_gamut(r, g, b, a, rXgXbX, rYgYbY, rZgZbZ, dr, dg, db, da);
            translate_gamut(rTgTbT, dr, dg, db);

            if (kPremul_SkAlphaType == kAlphaType) {
                premultiply(dr, dg, db, da);
            }

            load(src, r, g, b, a, srcTables);
            src += 4;
            len -= 4;

            store(dst, src - 4, dr, dg, db, da, dstTables, kSwapRB);
            dst = SkTAddOffset<void>(dst, 4 * sizeOfDstPixel);
        }

        transform_gamut(r, g, b, a, rXgXbX, rYgYbY, rZgZbZ, dr, dg, db, da);
        translate_gamut(rTgTbT, dr, dg, db);

        if (kPremul_SkAlphaType == kAlphaType) {
            premultiply(dr, dg, db, da);
        }

        store(dst, src - 4, dr, dg, db, da, dstTables, kSwapRB);
        dst = SkTAddOffset<void>(dst, 4 * sizeOfDstPixel);
    }

    while (len > 0) {
        Sk4f r, g, b, a;
        load_1(src, r, g, b, a, srcTables);

        Sk4f rgba;
        transform_gamut_1(r, g, b, rXgXbX, rYgYbY, rZgZbZ, rgba);

        translate_gamut_1(rTgTbT, rgba);

        store_1(dst, src, rgba, a, dstTables, kSwapRB);

        src += 1;
        len -= 1;
        dst = SkTAddOffset<void>(dst, sizeOfDstPixel);
    }
}

#endif // SkColorSpaceXformOpts_DEFINED
