/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include "SkHalf.h"
#include "SkPM4f.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"

using Kernel_Sk4f = void(void*, size_t, size_t, Sk4f&, Sk4f&, Sk4f&, Sk4f&,
                                                Sk4f&, Sk4f&, Sk4f&, Sk4f&);

// These are always static, and we _really_ want them to inline.
// If you find yourself wanting a non-inline stage, write a SkRasterPipeline::Fn directly.
#define KERNEL_Sk4f(name)                                                      \
    static SK_ALWAYS_INLINE void name(void* ctx, size_t x, size_t tail,        \
                                      Sk4f&  r, Sk4f&  g, Sk4f&  b, Sk4f&  a,  \
                                      Sk4f& dr, Sk4f& dg, Sk4f& db, Sk4f& da)


template <Kernel_Sk4f kernel, bool kCallNext>
static inline void SK_VECTORCALL stage_4(SkRasterPipeline::Stage* st, size_t x, size_t tail,
                                         Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                         Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
    // Passing 0 lets the optimizer completely drop any "if (tail) {...}" code in kernel.
    kernel(st->ctx<void*>(), x,0, r,g,b,a, dr,dg,db,da);
    if (kCallNext) {
        st->next(x,tail, r,g,b,a, dr,dg,db,da);  // It's faster to pass t here than 0.
    }
}

template <Kernel_Sk4f kernel, bool kCallNext>
static inline void SK_VECTORCALL stage_1_3(SkRasterPipeline::Stage* st, size_t x, size_t tail,
                                           Sk4f  r, Sk4f  g, Sk4f  b, Sk4f  a,
                                           Sk4f dr, Sk4f dg, Sk4f db, Sk4f da) {
#if defined(__clang__)
    __builtin_assume(tail > 0);  // This flourish lets Clang compile away any tail==0 code.
#endif
    kernel(st->ctx<void*>(), x,tail, r,g,b,a, dr,dg,db,da);
    if (kCallNext) {
        st->next(x,tail, r,g,b,a, dr,dg,db,da);
    }
}

namespace SK_OPTS_NS {

    // Clamp colors into [0,1] premul (e.g. just before storing back to memory).
    static void clamp_01_premul(Sk4f& r, Sk4f& g, Sk4f& b, Sk4f& a) {
        a = Sk4f::Max(a, 0.0f);
        r = Sk4f::Max(r, 0.0f);
        g = Sk4f::Max(g, 0.0f);
        b = Sk4f::Max(b, 0.0f);

        a = Sk4f::Min(a, 1.0f);
        r = Sk4f::Min(r, a);
        g = Sk4f::Min(g, a);
        b = Sk4f::Min(b, a);
    }

    static Sk4f lerp(const Sk4f& from, const Sk4f& to, const Sk4f& cov) {
        return from + (to-from)*cov;
    }

    template <typename T>
    static SkNx<4,T> load_tail(size_t tail, const T* src) {
        if (tail) {
           return SkNx<4,T>(src[0], (tail>1 ? src[1] : 0), (tail>2 ? src[2] : 0), 0);
        }
        return SkNx<4,T>::Load(src);
    }

    template <typename T>
    static void store_tail(size_t tail, const SkNx<4,T>& v, T* dst) {
        switch(tail) {
            case 0: return v.store(dst);
            case 3: dst[2] = v[2];
            case 2: dst[1] = v[1];
            case 1: dst[0] = v[0];
        }
    }

    static void from_565(const Sk4h& _565, Sk4f* r, Sk4f* g, Sk4f* b) {
        Sk4i _32_bit = SkNx_cast<int>(_565);

        *r = SkNx_cast<float>(_32_bit & SK_R16_MASK_IN_PLACE) * (1.0f / SK_R16_MASK_IN_PLACE);
        *g = SkNx_cast<float>(_32_bit & SK_G16_MASK_IN_PLACE) * (1.0f / SK_G16_MASK_IN_PLACE);
        *b = SkNx_cast<float>(_32_bit & SK_B16_MASK_IN_PLACE) * (1.0f / SK_B16_MASK_IN_PLACE);
    }

    static Sk4h to_565(const Sk4f& r, const Sk4f& g, const Sk4f& b) {
        return SkNx_cast<uint16_t>( Sk4f_round(r * SK_R16_MASK) << SK_R16_SHIFT
                                  | Sk4f_round(g * SK_G16_MASK) << SK_G16_SHIFT
                                  | Sk4f_round(b * SK_B16_MASK) << SK_B16_SHIFT);
    }


    // The default shader produces a constant color (from the SkPaint).
    KERNEL_Sk4f(constant_color) {
        auto color = (const SkPM4f*)ctx;
        r = color->r();
        g = color->g();
        b = color->b();
        a = color->a();
    }

    // The default transfer mode is srcover, s' = s + d*(1-sa).
    KERNEL_Sk4f(srcover) {
        r += dr*(1.0f - a);
        g += dg*(1.0f - a);
        b += db*(1.0f - a);
        a += da*(1.0f - a);
    }

    // s' = d(1-c) + sc, for a constant c.
    KERNEL_Sk4f(lerp_constant_float) {
        Sk4f c = *(const float*)ctx;

        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }

    // s' = sc for 8-bit c.
    KERNEL_Sk4f(scale_u8) {
        auto ptr = (const uint8_t*)ctx + x;

        Sk4f c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
        r = r*c;
        g = g*c;
        b = b*c;
        a = a*c;
    }

    // s' = d(1-c) + sc for 8-bit c.
    KERNEL_Sk4f(lerp_u8) {
        auto ptr = (const uint8_t*)ctx + x;

        Sk4f c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }

    // s' = d(1-c) + sc for 565 c.
    KERNEL_Sk4f(lerp_565) {
        auto ptr = (const uint16_t*)ctx + x;
        Sk4f cr, cg, cb;
        from_565(load_tail(tail, ptr), &cr, &cg, &cb);

        r = lerp(dr, r, cr);
        g = lerp(dg, g, cg);
        b = lerp(db, b, cb);
        a = 1.0f;
    }

    KERNEL_Sk4f(load_d_565) {
        auto ptr = (const uint16_t*)ctx + x;
        from_565(load_tail(tail, ptr), &dr,&dg,&db);
        da = 1.0f;
    }

    KERNEL_Sk4f(load_s_565) {
        auto ptr = (const uint16_t*)ctx + x;
        from_565(load_tail(tail, ptr), &r,&g,&b);
        a = 1.0f;
    }

    KERNEL_Sk4f(store_565) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint16_t*)ctx + x;
        store_tail(tail, to_565(r,g,b), ptr);
    }

    KERNEL_Sk4f(load_d_f16) {
        auto ptr = (const uint64_t*)ctx + x;

        if (tail) {
            auto p0 =          SkHalfToFloat_finite_ftz(ptr[0])          ,
                 p1 = tail>1 ? SkHalfToFloat_finite_ftz(ptr[1]) : Sk4f{0},
                 p2 = tail>2 ? SkHalfToFloat_finite_ftz(ptr[2]) : Sk4f{0};
            dr = { p0[0],p1[0],p2[0],0 };
            dg = { p0[1],p1[1],p2[1],0 };
            db = { p0[2],p1[2],p2[2],0 };
            da = { p0[3],p1[3],p2[3],0 };
            return;
        }

        Sk4h rh, gh, bh, ah;
        Sk4h_load4(ptr, &rh, &gh, &bh, &ah);
        dr = SkHalfToFloat_finite_ftz(rh);
        dg = SkHalfToFloat_finite_ftz(gh);
        db = SkHalfToFloat_finite_ftz(bh);
        da = SkHalfToFloat_finite_ftz(ah);
    }

    KERNEL_Sk4f(load_s_f16) {
        auto ptr = (const uint64_t*)ctx + x;

        if (tail) {
            auto p0 =          SkHalfToFloat_finite_ftz(ptr[0])          ,
                 p1 = tail>1 ? SkHalfToFloat_finite_ftz(ptr[1]) : Sk4f{0},
                 p2 = tail>2 ? SkHalfToFloat_finite_ftz(ptr[2]) : Sk4f{0};
            r = { p0[0],p1[0],p2[0],0 };
            g = { p0[1],p1[1],p2[1],0 };
            b = { p0[2],p1[2],p2[2],0 };
            a = { p0[3],p1[3],p2[3],0 };
            return;
        }

        Sk4h rh, gh, bh, ah;
        Sk4h_load4(ptr, &rh, &gh, &bh, &ah);
        r = SkHalfToFloat_finite_ftz(rh);
        g = SkHalfToFloat_finite_ftz(gh);
        b = SkHalfToFloat_finite_ftz(bh);
        a = SkHalfToFloat_finite_ftz(ah);
    }

    KERNEL_Sk4f(store_f16) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint64_t*)ctx + x;

        switch (tail) {
            case 0: return Sk4h_store4(ptr, SkFloatToHalf_finite_ftz(r),
                                            SkFloatToHalf_finite_ftz(g),
                                            SkFloatToHalf_finite_ftz(b),
                                            SkFloatToHalf_finite_ftz(a));

            case 3: SkFloatToHalf_finite_ftz({r[2], g[2], b[2], a[2]}).store(ptr+2);
            case 2: SkFloatToHalf_finite_ftz({r[1], g[1], b[1], a[1]}).store(ptr+1);
            case 1: SkFloatToHalf_finite_ftz({r[0], g[0], b[0], a[0]}).store(ptr+0);
        }
    }


    // Load 8-bit SkPMColor-order sRGB.
    KERNEL_Sk4f(load_d_srgb) {
        auto ptr = (const uint32_t*)ctx + x;

        if (tail) {
            float rs[] = {0,0,0,0},
                  gs[] = {0,0,0,0},
                  bs[] = {0,0,0,0},
                  as[] = {0,0,0,0};
            for (size_t i = 0; i < tail; i++) {
                rs[i] = sk_linear_from_srgb[(ptr[i] >> SK_R32_SHIFT) & 0xff];
                gs[i] = sk_linear_from_srgb[(ptr[i] >> SK_G32_SHIFT) & 0xff];
                bs[i] = sk_linear_from_srgb[(ptr[i] >> SK_B32_SHIFT) & 0xff];
                as[i] =       (1/255.0f) *  (ptr[i] >> SK_A32_SHIFT)        ;
            }
            dr = Sk4f::Load(rs);
            dg = Sk4f::Load(gs);
            db = Sk4f::Load(bs);
            da = Sk4f::Load(as);
            return;
        }

        dr = { sk_linear_from_srgb[(ptr[0] >> SK_R32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[1] >> SK_R32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[2] >> SK_R32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[3] >> SK_R32_SHIFT) & 0xff] };

        dg = { sk_linear_from_srgb[(ptr[0] >> SK_G32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[1] >> SK_G32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[2] >> SK_G32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[3] >> SK_G32_SHIFT) & 0xff] };

        db = { sk_linear_from_srgb[(ptr[0] >> SK_B32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[1] >> SK_B32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[2] >> SK_B32_SHIFT) & 0xff],
               sk_linear_from_srgb[(ptr[3] >> SK_B32_SHIFT) & 0xff] };

        da = SkNx_cast<float>(Sk4u::Load(ptr) >> SK_A32_SHIFT) * (1/255.0f);
    }

    KERNEL_Sk4f(load_s_srgb) {
        auto ptr = (const uint32_t*)ctx + x;

        if (tail) {
            float rs[] = {0,0,0,0},
                  gs[] = {0,0,0,0},
                  bs[] = {0,0,0,0},
                  as[] = {0,0,0,0};
            for (size_t i = 0; i < tail; i++) {
                rs[i] = sk_linear_from_srgb[(ptr[i] >> SK_R32_SHIFT) & 0xff];
                gs[i] = sk_linear_from_srgb[(ptr[i] >> SK_G32_SHIFT) & 0xff];
                bs[i] = sk_linear_from_srgb[(ptr[i] >> SK_B32_SHIFT) & 0xff];
                as[i] =       (1/255.0f) *  (ptr[i] >> SK_A32_SHIFT)        ;
            }
            r = Sk4f::Load(rs);
            g = Sk4f::Load(gs);
            b = Sk4f::Load(bs);
            a = Sk4f::Load(as);
            return;
        }

        r = { sk_linear_from_srgb[(ptr[0] >> SK_R32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[1] >> SK_R32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[2] >> SK_R32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[3] >> SK_R32_SHIFT) & 0xff] };

        g = { sk_linear_from_srgb[(ptr[0] >> SK_G32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[1] >> SK_G32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[2] >> SK_G32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[3] >> SK_G32_SHIFT) & 0xff] };

        b = { sk_linear_from_srgb[(ptr[0] >> SK_B32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[1] >> SK_B32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[2] >> SK_B32_SHIFT) & 0xff],
              sk_linear_from_srgb[(ptr[3] >> SK_B32_SHIFT) & 0xff] };

        a = SkNx_cast<float>(Sk4u::Load(ptr) >> SK_A32_SHIFT) * (1/255.0f);
    }

    KERNEL_Sk4f(store_srgb) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint32_t*)ctx + x;
        store_tail(tail, ( sk_linear_to_srgb_noclamp(r) << SK_R32_SHIFT
                         | sk_linear_to_srgb_noclamp(g) << SK_G32_SHIFT
                         | sk_linear_to_srgb_noclamp(b) << SK_B32_SHIFT
                         |       Sk4f_round(255.0f * a) << SK_A32_SHIFT), (int*)ptr);
    }

}

#endif//SkRasterPipeline_opts_DEFINED
