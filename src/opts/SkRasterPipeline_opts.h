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

using SkNf = SkRasterPipeline::V;
static constexpr auto N = sizeof(SkNf) / sizeof(float);
using SkNi = SkNx<N, int>;
using SkNh = SkNx<N, uint16_t>;

using Kernel = void(void*, size_t, size_t, SkNf&, SkNf&, SkNf&, SkNf&,
                                           SkNf&, SkNf&, SkNf&, SkNf&);

// These are always static, and we _really_ want them to inline.
// If you find yourself wanting a non-inline stage, write a SkRasterPipeline::Fn directly.
#define KERNEL(name)                                                          \
    static SK_ALWAYS_INLINE void name(void* ctx, size_t x, size_t tail,       \
                                      SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a, \
                                      SkNf& dr, SkNf& dg, SkNf& db, SkNf& da)


template <Kernel kernel, bool kCallNext>
static inline void SK_VECTORCALL stage_b(SkRasterPipeline::Stage* st, size_t x, size_t tail,
                                         SkNf  r, SkNf  g, SkNf  b, SkNf  a,
                                         SkNf dr, SkNf dg, SkNf db, SkNf da) {
    // Passing 0 lets the optimizer completely drop any "if (tail) {...}" code in kernel.
    kernel(st->ctx<void*>(), x,0, r,g,b,a, dr,dg,db,da);
    if (kCallNext) {
        st->next(x,tail, r,g,b,a, dr,dg,db,da);  // It's faster to pass tail here than 0.
    }
}

template <Kernel kernel, bool kCallNext>
static inline void SK_VECTORCALL stage_t(SkRasterPipeline::Stage* st, size_t x, size_t tail,
                                         SkNf  r, SkNf  g, SkNf  b, SkNf  a,
                                         SkNf dr, SkNf dg, SkNf db, SkNf da) {
#if defined(__clang__)
    __builtin_assume(tail > 0);  // This flourish lets Clang compile away any tail==0 code.
#endif
    kernel(st->ctx<void*>(), x,tail, r,g,b,a, dr,dg,db,da);
    if (kCallNext) {
        st->next(x,tail, r,g,b,a, dr,dg,db,da);
    }
}

// Many xfermodes apply the same logic to each channel.
#define RGBA_XFERMODE(name)                                                                     \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,                   \
                                               const SkNf& d, const SkNf& da);                  \
    static void SK_VECTORCALL name(SkRasterPipeline::Stage* st, size_t x, size_t tail,          \
                                   SkNf  r, SkNf  g, SkNf  b, SkNf  a,                          \
                                   SkNf dr, SkNf dg, SkNf db, SkNf da) {                        \
        r = name##_kernel(r,a,dr,da);                                                           \
        g = name##_kernel(g,a,dg,da);                                                           \
        b = name##_kernel(b,a,db,da);                                                           \
        a = name##_kernel(a,a,da,da);                                                           \
        st->next(x,tail, r,g,b,a, dr,dg,db,da);                                                 \
    }                                                                                           \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,                   \
                                               const SkNf& d, const SkNf& da)

// Most of the rest apply the same logic to color channels and use srcover's alpha logic.
#define RGB_XFERMODE(name)                                                                      \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,                   \
                                               const SkNf& d, const SkNf& da);                  \
    static void SK_VECTORCALL name(SkRasterPipeline::Stage* st, size_t x, size_t tail,          \
                                   SkNf  r, SkNf  g, SkNf  b, SkNf  a,                          \
                                   SkNf dr, SkNf dg, SkNf db, SkNf da) {                        \
        r = name##_kernel(r,a,dr,da);                                                           \
        g = name##_kernel(g,a,dg,da);                                                           \
        b = name##_kernel(b,a,db,da);                                                           \
        a = a + (da * (1.0f-a));                                                                \
        st->next(x,tail, r,g,b,a, dr,dg,db,da);                                                 \
    }                                                                                           \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,                   \
                                               const SkNf& d, const SkNf& da)


namespace SK_OPTS_NS {

    static inline void run_pipeline(size_t x, size_t n,
                                    SkRasterPipeline::Fn bodyStart, SkRasterPipeline::Stage* body,
                                    SkRasterPipeline::Fn tailStart, SkRasterPipeline::Stage* tail) {
        SkNf v;  // Fastest to start uninitialized.
        while (n >= N) {
            bodyStart(body, x,0, v,v,v,v, v,v,v,v);
            x += N;
            n -= N;
        }
        if (n > 0) {
            tailStart(tail, x,n, v,v,v,v, v,v,v,v);
        }
    }

    // Clamp colors into [0,1] premul (e.g. just before storing back to memory).
    static void clamp_01_premul(SkNf& r, SkNf& g, SkNf& b, SkNf& a) {
        a = SkNf::Max(a, 0.0f);
        r = SkNf::Max(r, 0.0f);
        g = SkNf::Max(g, 0.0f);
        b = SkNf::Max(b, 0.0f);

        a = SkNf::Min(a, 1.0f);
        r = SkNf::Min(r, a);
        g = SkNf::Min(g, a);
        b = SkNf::Min(b, a);
    }

    static SkNf inv(const SkNf& x) { return 1.0f - x; }

    static SkNf lerp(const SkNf& from, const SkNf& to, const SkNf& cov) {
        return from + (to-from)*cov;
    }

    template <typename T>
    static SkNx<N,T> load_tail(size_t tail, const T* src) {
        if (tail) {
            // TODO: maskload for 32- and 64-bit T
            T vals[N] = { 0 };
            for (size_t i = 0; i < tail; i++) {
                vals[i] = src[i];
            }
            return SkNx<N,T>::Load(vals);
        }
        return SkNx<N,T>::Load(src);
    }

    template <typename T>
    static void store_tail(size_t tail, const SkNx<N,T>& v, T* dst) {
        switch(tail) {
            case 0: return v.store(dst);

            // TODO: maskstore for 32- and 64-bit T
            case 7: dst[6] = v[6];
            case 6: dst[5] = v[5];
            case 5: dst[4] = v[4];
            case 4: dst[3] = v[3];
            case 3: dst[2] = v[2];
            case 2: dst[1] = v[1];
            case 1: dst[0] = v[0];
        }
    }

    static void from_565(const SkNh& _565, SkNf* r, SkNf* g, SkNf* b) {
        auto _32_bit = SkNx_cast<int>(_565);

        *r = SkNx_cast<float>(_32_bit & SK_R16_MASK_IN_PLACE) * (1.0f / SK_R16_MASK_IN_PLACE);
        *g = SkNx_cast<float>(_32_bit & SK_G16_MASK_IN_PLACE) * (1.0f / SK_G16_MASK_IN_PLACE);
        *b = SkNx_cast<float>(_32_bit & SK_B16_MASK_IN_PLACE) * (1.0f / SK_B16_MASK_IN_PLACE);
    }

    static SkNh to_565(const SkNf& r, const SkNf& g, const SkNf& b) {
        return SkNx_cast<uint16_t>( SkNx_cast<int>(r * SK_R16_MASK + 0.5f) << SK_R16_SHIFT
                                  | SkNx_cast<int>(g * SK_G16_MASK + 0.5f) << SK_G16_SHIFT
                                  | SkNx_cast<int>(b * SK_B16_MASK + 0.5f) << SK_B16_SHIFT);
    }

    KERNEL(just_return) { }

    // The default shader produces a constant color (from the SkPaint).
    KERNEL(constant_color) {
        auto color = (const SkPM4f*)ctx;
        r = color->r();
        g = color->g();
        b = color->b();
        a = color->a();
    }

    // s' = d(1-c) + sc, for a constant c.
    KERNEL(lerp_constant_float) {
        SkNf c = *(const float*)ctx;

        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }

    // s' = sc for 8-bit c.
    KERNEL(scale_u8) {
        auto ptr = (const uint8_t*)ctx + x;

        SkNf c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
        r = r*c;
        g = g*c;
        b = b*c;
        a = a*c;
    }

    // s' = d(1-c) + sc for 8-bit c.
    KERNEL(lerp_u8) {
        auto ptr = (const uint8_t*)ctx + x;

        SkNf c = SkNx_cast<float>(load_tail(tail, ptr)) * (1/255.0f);
        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }

    // s' = d(1-c) + sc for 565 c.
    KERNEL(lerp_565) {
        auto ptr = (const uint16_t*)ctx + x;
        SkNf cr, cg, cb;
        from_565(load_tail(tail, ptr), &cr, &cg, &cb);

        r = lerp(dr, r, cr);
        g = lerp(dg, g, cg);
        b = lerp(db, b, cb);
        a = 1.0f;
    }

    KERNEL(load_d_565) {
        auto ptr = (const uint16_t*)ctx + x;
        from_565(load_tail(tail, ptr), &dr,&dg,&db);
        da = 1.0f;
    }

    KERNEL(load_s_565) {
        auto ptr = (const uint16_t*)ctx + x;
        from_565(load_tail(tail, ptr), &r,&g,&b);
        a = 1.0f;
    }

    KERNEL(store_565) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint16_t*)ctx + x;
        store_tail(tail, to_565(r,g,b), ptr);
    }

    KERNEL(load_d_f16) {
        /*
        auto ptr = (const uint64_t*)ctx + x;

        if (tail) {
            auto p0 =          SkHalfToFloat_finite_ftz(ptr[0])          ,
                 p1 = tail>1 ? SkHalfToFloat_finite_ftz(ptr[1]) : SkNf{0},
                 p2 = tail>2 ? SkHalfToFloat_finite_ftz(ptr[2]) : SkNf{0};
            dr = { p0[0],p1[0],p2[0],0 };
            dg = { p0[1],p1[1],p2[1],0 };
            db = { p0[2],p1[2],p2[2],0 };
            da = { p0[3],p1[3],p2[3],0 };
            return;
        }

        SkNh rh, gh, bh, ah;
        Sk4h_load4(ptr, &rh, &gh, &bh, &ah);
        dr = SkHalfToFloat_finite_ftz(rh);
        dg = SkHalfToFloat_finite_ftz(gh);
        db = SkHalfToFloat_finite_ftz(bh);
        da = SkHalfToFloat_finite_ftz(ah);
        */
    }

    KERNEL(load_s_f16) {
        /*
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
        */
    }

    KERNEL(store_f16) {
        /*
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
        */
    }


    // Load 8-bit SkPMColor-order sRGB.
    KERNEL(load_d_srgb) {
        auto ptr = (const uint32_t*)ctx + x;

        auto px = load_tail(tail, (const int*)ptr);
        dr =    sk_linear_from_srgb_math((px >> SK_R32_SHIFT) & 0xff);
        dg =    sk_linear_from_srgb_math((px >> SK_G32_SHIFT) & 0xff);
        db =    sk_linear_from_srgb_math((px >> SK_B32_SHIFT) & 0xff);
        da = (1/255.0f)*SkNx_cast<float>((px >> SK_A32_SHIFT) & 0xff);
    }

    KERNEL(load_s_srgb) {
        auto ptr = (const uint32_t*)ctx + x;

        auto px = load_tail(tail, (const int*)ptr);
        r =    sk_linear_from_srgb_math((px >> SK_R32_SHIFT) & 0xff);
        g =    sk_linear_from_srgb_math((px >> SK_G32_SHIFT) & 0xff);
        b =    sk_linear_from_srgb_math((px >> SK_B32_SHIFT) & 0xff);
        a = (1/255.0f)*SkNx_cast<float>((px >> SK_A32_SHIFT) & 0xff);
    }

    KERNEL(store_srgb) {
        clamp_01_premul(r,g,b,a);
        auto ptr = (uint32_t*)ctx + x;
        store_tail(tail, (      sk_linear_to_srgb_noclamp(r) << SK_R32_SHIFT
                         |      sk_linear_to_srgb_noclamp(g) << SK_G32_SHIFT
                         |      sk_linear_to_srgb_noclamp(b) << SK_B32_SHIFT
                         | SkNx_cast<int>(255.0f * a + 0.5f) << SK_A32_SHIFT), (int*)ptr);
    }

    RGBA_XFERMODE(clear)    { return 0.0f; }
  //RGBA_XFERMODE(src)      { return s; }   // This would be a no-op stage, so we just omit it.
    RGBA_XFERMODE(dst)      { return d; }

    RGBA_XFERMODE(srcatop)  { return s*da + d*inv(sa); }
    RGBA_XFERMODE(srcin)    { return s * da; }
    RGBA_XFERMODE(srcout)   { return s * inv(da); }
    RGBA_XFERMODE(srcover)  { return s + inv(sa)*d; }
    RGBA_XFERMODE(dstatop)  { return srcatop_kernel(d,da,s,sa); }
    RGBA_XFERMODE(dstin)    { return srcin_kernel  (d,da,s,sa); }
    RGBA_XFERMODE(dstout)   { return srcout_kernel (d,da,s,sa); }
    RGBA_XFERMODE(dstover)  { return srcover_kernel(d,da,s,sa); }

    RGBA_XFERMODE(modulate) { return s*d; }
    RGBA_XFERMODE(multiply) { return s*inv(da) + d*inv(sa) + s*d; }
    RGBA_XFERMODE(plus_)    { return s + d; }
    RGBA_XFERMODE(screen)   { return s + d - s*d; }
    RGBA_XFERMODE(xor_)     { return s*inv(da) + d*inv(sa); }

    RGB_XFERMODE(colorburn) {
        return (d == da  ).thenElse(d + s*inv(da),
               (s == 0.0f).thenElse(s + d*inv(sa),
                                    sa*(da - SkNf::Min(da, (da-d)*sa/s)) + s*inv(da) + d*inv(sa)));
    }
    RGB_XFERMODE(colordodge) {
        return (d == 0.0f).thenElse(d + s*inv(da),
               (s == sa  ).thenElse(s + d*inv(sa),
                                    sa*SkNf::Min(da, (d*sa)/(sa - s)) + s*inv(da) + d*inv(sa)));
    }
    RGB_XFERMODE(darken)     { return s + d - SkNf::Max(s*da, d*sa); }
    RGB_XFERMODE(difference) { return s + d - 2.0f*SkNf::Min(s*da,d*sa); }
    RGB_XFERMODE(exclusion)  { return s + d - 2.0f*s*d; }
    RGB_XFERMODE(hardlight) {
        return s*inv(da) + d*inv(sa)
             + (2.0f*s <= sa).thenElse(2.0f*s*d, sa*da - 2.0f*(da-d)*(sa-s));
    }
    RGB_XFERMODE(lighten) { return s + d - SkNf::Min(s*da, d*sa); }
    RGB_XFERMODE(overlay) { return hardlight_kernel(d,da,s,sa); }
    RGB_XFERMODE(softlight) {
        SkNf m  = (da > 0.0f).thenElse(d / da, 0.0f),
             s2 = 2.0f*s,
             m4 = 4.0f*m;

        // The logic forks three ways:
        //    1. dark src?
        //    2. light src, dark dst?
        //    3. light src, light dst?
        SkNf darkSrc = d*(sa + (s2 - sa)*(1.0f - m)),     // Used in case 1.
             darkDst = (m4*m4 + m4)*(m - 1.0f) + 7.0f*m,  // Used in case 2.
             liteDst = m.rsqrt().invert() - m,            // Used in case 3.
             liteSrc = d*sa + da*(s2 - sa) * (4.0f*d <= da).thenElse(darkDst, liteDst);  // 2 or 3?
        return s*inv(da) + d*inv(sa) + (s2 <= sa).thenElse(darkSrc, liteSrc);  // 1 or (2 or 3)?
    }
}

#undef KERNEL
#undef RGBA_XFERMODE
#undef RGB_XFERMODE

#endif//SkRasterPipeline_opts_DEFINED
