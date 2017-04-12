/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include "SkColorPriv.h"
#include "SkColorLookUpTable.h"
#include "SkColorSpaceXform_A2B.h"
#include "SkColorSpaceXformPriv.h"
#include "SkHalf.h"
#include "SkMSAN.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkShader.h"
#include "SkSRGB.h"
#include "../jumper/SkJumper.h"

namespace {

    static constexpr int N = 4;

    using SkNf = SkNx<N, float>;
    using SkNi = SkNx<N, int32_t>;
    using SkNu = SkNx<N, uint32_t>;
    using SkNh = SkNx<N, uint16_t>;
    using SkNb = SkNx<N, uint8_t>;

    using Fn = void(SK_VECTORCALL *)(size_t x_tail, void** p, SkNf,SkNf,SkNf,SkNf,
                                                              SkNf,SkNf,SkNf,SkNf);
    // x_tail encodes two values x and tail as x*N+tail, where 0 <= tail < N.
    // x is the induction variable we're walking along, incrementing by N each step.
    // tail == 0 means work with a full N pixels; otherwise use only the low tail pixels.
    //
    // p is our program, a sequence of Fn to call interlaced with any void* context pointers.  E.g.
    //    &load_8888
    //    (src ptr)
    //    &from_srgb
    //    &move_src_dst
    //    &load_f16
    //    (dst ptr)
    //    &swap
    //    &srcover
    //    &store_f16
    //    (dst ptr)
    //    &just_return

}  // namespace

#define SI static inline

// Basically, return *(*ptr)++, maybe faster than the compiler can do it.
SI void* load_and_increment(void*** ptr) {
    // We do this often enough that it's worth hyper-optimizing.
    // x86 can do this in one instruction if ptr is in rsi.
    // (This is why p is the second argument to Fn: it's passed in rsi.)
#if defined(__GNUC__) && defined(__x86_64__)
    void* rax;
    __asm__("lodsq" : "=a"(rax), "+S"(*ptr));
    return rax;
#else
    return *(*ptr)++;
#endif
}

// Stages are logically a pipeline, and physically are contiguous in an array.
// To get to the next stage, we just increment our pointer to the next array element.
SI void SK_VECTORCALL next(size_t x_tail, void** p, SkNf  r, SkNf  g, SkNf  b, SkNf  a,
                                                    SkNf dr, SkNf dg, SkNf db, SkNf da) {
    auto next = (Fn)load_and_increment(&p);
    next(x_tail,p, r,g,b,a, dr,dg,db,da);
}

// Stages defined below always call next.
// This is always the last stage, a backstop that actually returns to the caller when done.
SI void SK_VECTORCALL just_return(size_t, void**, SkNf, SkNf, SkNf, SkNf,
                                                  SkNf, SkNf, SkNf, SkNf) {}

#define STAGE(name)                                                                      \
    static SK_ALWAYS_INLINE void name##_kernel(size_t x, size_t tail,                    \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,   \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da);  \
    SI void SK_VECTORCALL name(size_t x_tail, void** p,                                  \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,                       \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {                     \
        name##_kernel(x_tail/N, x_tail%N, r,g,b,a, dr,dg,db,da);                         \
        next(x_tail,p, r,g,b,a, dr,dg,db,da);                                            \
    }                                                                                    \
    static SK_ALWAYS_INLINE void name##_kernel(size_t x, size_t tail,                    \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,   \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da)

#define STAGE_CTX(name, Ctx)                                                             \
    static SK_ALWAYS_INLINE void name##_kernel(Ctx ctx, size_t x, size_t tail,           \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,   \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da);  \
    SI void SK_VECTORCALL name(size_t x_tail, void** p,                                  \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,                       \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {                     \
        auto ctx = (Ctx)load_and_increment(&p);                                          \
        name##_kernel(ctx, x_tail/N, x_tail%N, r,g,b,a, dr,dg,db,da);                    \
        next(x_tail,p, r,g,b,a, dr,dg,db,da);                                            \
    }                                                                                    \
    static SK_ALWAYS_INLINE void name##_kernel(Ctx ctx, size_t x, size_t tail,           \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,   \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da)

// Many xfermodes apply the same logic to each channel.
#define RGBA_XFERMODE(name)                                                     \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da);  \
    SI void SK_VECTORCALL name(size_t x_tail, void** p,                         \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,              \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {            \
        r = name##_kernel(r,a,dr,da);                                           \
        g = name##_kernel(g,a,dg,da);                                           \
        b = name##_kernel(b,a,db,da);                                           \
        a = name##_kernel(a,a,da,da);                                           \
        next(x_tail,p, r,g,b,a, dr,dg,db,da);                                   \
    }                                                                           \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da)

// Most of the rest apply the same logic to color channels and use srcover's alpha logic.
#define RGB_XFERMODE(name)                                                      \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da);  \
    SI void SK_VECTORCALL name(size_t x_tail, void** p,                         \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,              \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {            \
        r = name##_kernel(r,a,dr,da);                                           \
        g = name##_kernel(g,a,dg,da);                                           \
        b = name##_kernel(b,a,db,da);                                           \
        a = a + (da * (1.0f-a));                                                \
        next(x_tail,p, r,g,b,a, dr,dg,db,da);                                   \
    }                                                                           \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da)

template <typename T>
SI SkNx<N,T> load(size_t tail, const T* src) {
    if (tail) {
        T buf[8];
        memset(buf, 0, 8*sizeof(T));
        switch (tail & (N-1)) {
            case 7: buf[6] = src[6];
            case 6: buf[5] = src[5];
            case 5: buf[4] = src[4];
            case 4: buf[3] = src[3];
            case 3: buf[2] = src[2];
            case 2: buf[1] = src[1];
        }
        buf[0] = src[0];
        return SkNx<N,T>::Load(buf);
    }
    return SkNx<N,T>::Load(src);
}
template <typename T>
SI SkNx<N,T> gather(size_t tail, const T* src, const SkNi& offset) {
    if (tail) {
        T buf[8] = {0};
        switch (tail & (N-1)) {
            case 7: buf[6] = src[offset[6]];
            case 6: buf[5] = src[offset[5]];
            case 5: buf[4] = src[offset[4]];
            case 4: buf[3] = src[offset[3]];
            case 3: buf[2] = src[offset[2]];
            case 2: buf[1] = src[offset[1]];
        }
        buf[0] = src[offset[0]];
        return SkNx<N,T>::Load(buf);
    }
    T buf[8];
    for (size_t i = 0; i < N; i++) {
        buf[i] = src[offset[i]];
    }
    return SkNx<N,T>::Load(buf);
}
template <typename T>
SI void store(size_t tail, const SkNx<N,T>& v, T* dst) {
    if (tail) {
        switch (tail & (N-1)) {
            case 7: dst[6] = v[6];
            case 6: dst[5] = v[5];
            case 5: dst[4] = v[4];
            case 4: dst[3] = v[3];
            case 3: dst[2] = v[2];
            case 2: dst[1] = v[1];
        }
        dst[0] = v[0];
        return;
    }
    v.store(dst);
}

SI SkNf SkNf_fma(const SkNf& f, const SkNf& m, const SkNf& a) { return SkNx_fma(f,m,a); }

SI SkNi SkNf_round(const SkNf& x, const SkNf& scale) {
    // Every time I try, _mm_cvtps_epi32 benches as slower than using FMA and _mm_cvttps_epi32.  :/
    return SkNx_cast<int>(SkNf_fma(x,scale, 0.5f));
}

SI SkNf SkNf_from_byte(const SkNi& x) {
    // Same trick as in store_8888: 0x470000BB == 32768.0f + BB/256.0f for all bytes BB.
    auto v = 0x47000000 | x;
    // Read this as (pun_float(v) - 32768.0f) * (256/255.0f), redistributed to be an FMA.
    return SkNf_fma(SkNf::Load(&v), 256/255.0f, -32768*256/255.0f);
}
SI SkNf SkNf_from_byte(const SkNu& x) { return SkNf_from_byte(SkNi::Load(&x)); }
SI SkNf SkNf_from_byte(const SkNb& x) { return SkNf_from_byte(SkNx_cast<int>(x)); }

SI void from_8888(const SkNu& _8888, SkNf* r, SkNf* g, SkNf* b, SkNf* a) {
    *r = SkNf_from_byte((_8888      ) & 0xff);
    *g = SkNf_from_byte((_8888 >>  8) & 0xff);
    *b = SkNf_from_byte((_8888 >> 16) & 0xff);
    *a = SkNf_from_byte((_8888 >> 24)       );
}
SI void from_4444(const SkNh& _4444, SkNf* r, SkNf* g, SkNf* b, SkNf* a) {
    auto _32_bit = SkNx_cast<int>(_4444);

    *r = SkNx_cast<float>(_32_bit & (0xF << SK_R4444_SHIFT)) * (1.0f / (0xF << SK_R4444_SHIFT));
    *g = SkNx_cast<float>(_32_bit & (0xF << SK_G4444_SHIFT)) * (1.0f / (0xF << SK_G4444_SHIFT));
    *b = SkNx_cast<float>(_32_bit & (0xF << SK_B4444_SHIFT)) * (1.0f / (0xF << SK_B4444_SHIFT));
    *a = SkNx_cast<float>(_32_bit & (0xF << SK_A4444_SHIFT)) * (1.0f / (0xF << SK_A4444_SHIFT));
}
SI void from_565(const SkNh& _565, SkNf* r, SkNf* g, SkNf* b) {
    auto _32_bit = SkNx_cast<int>(_565);

    *r = SkNx_cast<float>(_32_bit & SK_R16_MASK_IN_PLACE) * (1.0f / SK_R16_MASK_IN_PLACE);
    *g = SkNx_cast<float>(_32_bit & SK_G16_MASK_IN_PLACE) * (1.0f / SK_G16_MASK_IN_PLACE);
    *b = SkNx_cast<float>(_32_bit & SK_B16_MASK_IN_PLACE) * (1.0f / SK_B16_MASK_IN_PLACE);
}
SI void from_f16(const void* px, SkNf* r, SkNf* g, SkNf* b, SkNf* a) {
    SkNh rh, gh, bh, ah;
    SkNh::Load4(px, &rh, &gh, &bh, &ah);

    *r = SkHalfToFloat_finite_ftz(rh);
    *g = SkHalfToFloat_finite_ftz(gh);
    *b = SkHalfToFloat_finite_ftz(bh);
    *a = SkHalfToFloat_finite_ftz(ah);
}

STAGE(clamp_0) {
    a = SkNf::Max(a, 0.0f);
    r = SkNf::Max(r, 0.0f);
    g = SkNf::Max(g, 0.0f);
    b = SkNf::Max(b, 0.0f);
}
STAGE(clamp_1) {
    a = SkNf::Min(a, 1.0f);
    r = SkNf::Min(r, 1.0f);
    g = SkNf::Min(g, 1.0f);
    b = SkNf::Min(b, 1.0f);
}
STAGE(clamp_a) {
    a = SkNf::Min(a, 1.0f);
    r = SkNf::Min(r, a);
    g = SkNf::Min(g, a);
    b = SkNf::Min(b, a);
}

STAGE(unpremul) {
    auto scale = (a == 0.0f).thenElse(0.0f, 1.0f/a);
    r *= scale;
    g *= scale;
    b *= scale;
}
STAGE(premul) {
    r *= a;
    g *= a;
    b *= a;
}

STAGE_CTX(set_rgb, const float*) {
    r = ctx[0];
    g = ctx[1];
    b = ctx[2];
}
STAGE(swap_rb) { SkTSwap(r,b); }

STAGE(move_src_dst) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}
STAGE(move_dst_src) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}
STAGE(swap) {
    SkTSwap(r,dr);
    SkTSwap(g,dg);
    SkTSwap(b,db);
    SkTSwap(a,da);
}

STAGE(from_srgb) {
    r = sk_linear_from_srgb_math(r);
    g = sk_linear_from_srgb_math(g);
    b = sk_linear_from_srgb_math(b);
}
STAGE(to_srgb) {
    r = sk_linear_to_srgb_needs_round(r);
    g = sk_linear_to_srgb_needs_round(g);
    b = sk_linear_to_srgb_needs_round(b);
}

STAGE(from_2dot2) {
    auto from_2dot2 = [](const SkNf& x) {
        // x^(141/64) = x^(2.20312) is a great approximation of the true value, x^(2.2).
        // (note: x^(35/16) = x^(2.1875) is an okay one as well and would be quicker)
        auto x16 = x.rsqrt().rsqrt().rsqrt().rsqrt();   // x^(1/16) = x^(4/64);
        auto x64 = x16.rsqrt().rsqrt();                 // x^(1/64)

        // x^(141/64) = x^(128/64) * x^(12/64) * x^(1/64)
        return SkNf::Max((x*x) * (x16*x16*x16) * (x64), 0.0f);
    };

    r = from_2dot2(r);
    g = from_2dot2(g);
    b = from_2dot2(b);
}
STAGE(to_2dot2) {
    auto to_2dot2 = [](const SkNf& x) {
        // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
        auto x2  = x.rsqrt(),                            // x^(-1/2)
             x32 = x2.rsqrt().rsqrt().rsqrt().rsqrt(),   // x^(-1/32)
             x64 = x32.rsqrt();                          // x^(+1/64)

        // 29 = 32 - 2 - 1
        return SkNf::Max(x2.invert() * x32 * x64.invert(), 0.0f); // Watch out for NaN.
    };

    r = to_2dot2(r);
    g = to_2dot2(g);
    b = to_2dot2(b);
}

// The default shader produces a constant color (from the SkPaint).
STAGE_CTX(constant_color, const SkPM4f*) {
    r = ctx->r();
    g = ctx->g();
    b = ctx->b();
    a = ctx->a();
}

// Set up registers with values relevant to shaders.
STAGE_CTX(seed_shader, const int*) {
    int y = *ctx;

    static const float dx[] = { 0,1,2,3,4,5,6,7 };
    r = x + 0.5f + SkNf::Load(dx);  // dst pixel center x coordinates
    g = y + 0.5f;                   // dst pixel center y coordinate(s)
    b = 1.0f;
    a = 0.0f;
    dr = dg = db = da = 0.0f;
}

// s' = sc for a scalar c.
STAGE_CTX(scale_1_float, const float*) {
    SkNf c = *ctx;

    r *= c;
    g *= c;
    b *= c;
    a *= c;
}
// s' = sc for 8-bit c.
STAGE_CTX(scale_u8, const uint8_t**) {
    auto ptr = *ctx + x;
    SkNf c = SkNf_from_byte(load(tail, ptr));

    r = r*c;
    g = g*c;
    b = b*c;
    a = a*c;
}

SI SkNf lerp(const SkNf& from, const SkNf& to, const SkNf& cov) {
    return SkNf_fma(to-from, cov, from);
}

// s' = d(1-c) + sc, for a scalar c.
STAGE_CTX(lerp_1_float, const float*) {
    SkNf c = *ctx;

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// s' = d(1-c) + sc for 8-bit c.
STAGE_CTX(lerp_u8, const uint8_t**) {
    auto ptr = *ctx + x;
    SkNf c = SkNf_from_byte(load(tail, ptr));

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// s' = d(1-c) + sc for 565 c.
STAGE_CTX(lerp_565, const uint16_t**) {
    auto ptr = *ctx + x;
    SkNf cr, cg, cb;
    from_565(load(tail, ptr), &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0f;
}

STAGE_CTX(load_a8, const uint8_t**) {
    auto ptr = *ctx + x;
    r = g = b = 0.0f;
    a = SkNf_from_byte(load(tail, ptr));
}
STAGE_CTX(store_a8, uint8_t**) {
    auto ptr = *ctx + x;
    store(tail, SkNx_cast<uint8_t>(SkNf_round(255.0f, a)), ptr);
}

STAGE_CTX(load_g8, const uint8_t**) {
    auto ptr = *ctx + x;
    r = g = b = SkNf_from_byte(load(tail, ptr));
    a = 1.0f;
}

STAGE_CTX(load_565, const uint16_t**) {
    auto ptr = *ctx + x;
    from_565(load(tail, ptr), &r,&g,&b);
    a = 1.0f;
}
STAGE_CTX(store_565, uint16_t**) {
    auto ptr = *ctx + x;
    store(tail, SkNx_cast<uint16_t>( SkNf_round(r, SK_R16_MASK) << SK_R16_SHIFT
                                   | SkNf_round(g, SK_G16_MASK) << SK_G16_SHIFT
                                   | SkNf_round(b, SK_B16_MASK) << SK_B16_SHIFT), ptr);
}

STAGE_CTX(load_4444, const uint16_t**) {
    auto ptr = *ctx + x;
    from_4444(load(tail, ptr), &r,&g,&b,&a);
}
STAGE_CTX(store_4444, uint16_t**) {
    auto ptr = *ctx + x;
    store(tail, SkNx_cast<uint16_t>( SkNf_round(r, 0xF) << SK_R4444_SHIFT
                                   | SkNf_round(g, 0xF) << SK_G4444_SHIFT
                                   | SkNf_round(b, 0xF) << SK_B4444_SHIFT
                                   | SkNf_round(a, 0xF) << SK_A4444_SHIFT), ptr);
}

STAGE_CTX(load_f16, const uint64_t**) {
    auto ptr = *ctx + x;

    const void* src = ptr;
    SkNx<N, uint64_t> px;
    if (tail) {
        px = load(tail, ptr);
        src = &px;
    }
    from_f16(src, &r, &g, &b, &a);
}
STAGE_CTX(store_f16, uint64_t**) {
    auto ptr = *ctx + x;

    SkNx<N, uint64_t> px;
    SkNh::Store4(tail ? (void*)&px : (void*)ptr, SkFloatToHalf_finite_ftz(r),
                                                 SkFloatToHalf_finite_ftz(g),
                                                 SkFloatToHalf_finite_ftz(b),
                                                 SkFloatToHalf_finite_ftz(a));
    if (tail) {
        store(tail, px, ptr);
    }
}

STAGE_CTX(load_f32, const SkPM4f**) {
    auto ptr = *ctx + x;

    const void* src = ptr;
    SkNx<N, SkPM4f> px;
    if (tail) {
        px = load(tail, ptr);
        src = &px;
    }
    SkNf::Load4(src, &r, &g, &b, &a);
}
STAGE_CTX(store_f32, SkPM4f**) {
    auto ptr = *ctx + x;

    SkNx<N, SkPM4f> px;
    SkNf::Store4(tail ? (void*)&px : (void*)ptr, r,g,b,a);
    if (tail) {
        store(tail, px, ptr);
    }
}


STAGE_CTX(load_8888, const uint32_t**) {
    auto ptr = *ctx + x;
    from_8888(load(tail, ptr), &r, &g, &b, &a);
}
STAGE_CTX(store_8888, uint32_t**) {
    auto byte = [](const SkNf& x, int ix) {
        // Here's a neat trick: 0x47000000 == 32768.0f, and 0x470000ff == 32768.0f + (255/256.0f).
        auto v = SkNf_fma(255/256.0f, x, 32768.0f);
        switch (ix) {
            case 0: return SkNi::Load(&v) & 0xff;  // R
            case 3: return SkNi::Load(&v) << 24;   // A
        }
        return (SkNi::Load(&v) & 0xff) << (8*ix);  // B or G
    };

    auto ptr = *ctx + x;
    store(tail, byte(r,0)|byte(g,1)|byte(b,2)|byte(a,3), (int*)ptr);
}

STAGE_CTX(load_u16_be, const uint64_t**) {
    auto ptr = *ctx + x;
    const void* src = ptr;
    SkNx<N, uint64_t> px;
    if (tail) {
        px = load(tail, ptr);
        src = &px;
    }

    SkNh rh, gh, bh, ah;
    SkNh::Load4(src, &rh, &gh, &bh, &ah);
    r = (1.0f / 65535.0f) * SkNx_cast<float>((rh << 8) | (rh >> 8));
    g = (1.0f / 65535.0f) * SkNx_cast<float>((gh << 8) | (gh >> 8));
    b = (1.0f / 65535.0f) * SkNx_cast<float>((bh << 8) | (bh >> 8));
    a = (1.0f / 65535.0f) * SkNx_cast<float>((ah << 8) | (ah >> 8));
}

STAGE_CTX(load_rgb_u16_be, const uint16_t**) {
    auto ptr = *ctx + 3*x;
    const void* src = ptr;
    uint16_t buf[N*3] = {0};
    if (tail) {
        memcpy(buf, src, tail*3*sizeof(uint16_t));
        src = buf;
    }

    SkNh rh, gh, bh;
    SkNh::Load3(src, &rh, &gh, &bh);
    r = (1.0f / 65535.0f) * SkNx_cast<float>((rh << 8) | (rh >> 8));
    g = (1.0f / 65535.0f) * SkNx_cast<float>((gh << 8) | (gh >> 8));
    b = (1.0f / 65535.0f) * SkNx_cast<float>((bh << 8) | (bh >> 8));
    a = 1.0f;
}

STAGE_CTX(store_u16_be, uint64_t**) {
    auto to_u16_be = [](const SkNf& x) {
        SkNh x16 = SkNx_cast<uint16_t>(65535.0f * x);
        return (x16 << 8) | (x16 >> 8);
    };

    auto ptr = *ctx + x;
    SkNx<N, uint64_t> px;
    SkNh::Store4(tail ? (void*)&px : (void*)ptr, to_u16_be(r),
                                                 to_u16_be(g),
                                                 to_u16_be(b),
                                                 to_u16_be(a));
    if (tail) {
        store(tail, px, ptr);
    }
}

STAGE_CTX(load_tables, const LoadTablesContext*) {
    auto ptr = (const uint32_t*)ctx->fSrc + x;

    SkNu rgba = load(tail, ptr);
    auto to_int = [](const SkNu& v) { return SkNi::Load(&v); };
    r = gather(tail, ctx->fR, to_int((rgba >>  0) & 0xff));
    g = gather(tail, ctx->fG, to_int((rgba >>  8) & 0xff));
    b = gather(tail, ctx->fB, to_int((rgba >> 16) & 0xff));
    a = SkNf_from_byte(rgba >> 24);
}

STAGE_CTX(load_tables_u16_be, const LoadTablesContext*) {
    auto ptr = (const uint64_t*)ctx->fSrc + x;
    const void* src = ptr;
    SkNx<N, uint64_t> px;
    if (tail) {
        px = load(tail, ptr);
        src = &px;
    }

    SkNh rh, gh, bh, ah;
    SkNh::Load4(src, &rh, &gh, &bh, &ah);

    // ctx->fSrc is big-endian, so "& 0xff" grabs the 8 most significant bits of each component.
    r = gather(tail, ctx->fR, SkNx_cast<int>(rh & 0xff));
    g = gather(tail, ctx->fG, SkNx_cast<int>(gh & 0xff));
    b = gather(tail, ctx->fB, SkNx_cast<int>(bh & 0xff));
    a = (1.0f / 65535.0f) * SkNx_cast<float>((ah << 8) | (ah >> 8));
}

STAGE_CTX(load_tables_rgb_u16_be, const LoadTablesContext*) {
    auto ptr = (const uint16_t*)ctx->fSrc + 3*x;
    const void* src = ptr;
    uint16_t buf[N*3] = {0};
    if (tail) {
        memcpy(buf, src, tail*3*sizeof(uint16_t));
        src = buf;
    }

    SkNh rh, gh, bh;
    SkNh::Load3(src, &rh, &gh, &bh);

    // ctx->fSrc is big-endian, so "& 0xff" grabs the 8 most significant bits of each component.
    r = gather(tail, ctx->fR, SkNx_cast<int>(rh & 0xff));
    g = gather(tail, ctx->fG, SkNx_cast<int>(gh & 0xff));
    b = gather(tail, ctx->fB, SkNx_cast<int>(bh & 0xff));
    a = 1.0f;
}

SI SkNf inv(const SkNf& x) { return 1.0f - x; }

RGBA_XFERMODE(clear)    { return 0.0f; }
RGBA_XFERMODE(srcatop)  { return s*da + d*inv(sa); }
RGBA_XFERMODE(srcin)    { return s * da; }
RGBA_XFERMODE(srcout)   { return s * inv(da); }
RGBA_XFERMODE(srcover)  { return SkNf_fma(d, inv(sa), s); }
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

STAGE(luminance_to_alpha) {
    a = SK_LUM_COEFF_R*r + SK_LUM_COEFF_G*g + SK_LUM_COEFF_B*b;
    r = g = b = 0;
}

STAGE(rgb_to_hsl) {
    auto max = SkNf::Max(SkNf::Max(r, g), b);
    auto min = SkNf::Min(SkNf::Min(r, g), b);
    auto l = 0.5f * (max + min);

    auto d = max - min;
    auto d_inv = 1.0f/d;
    auto s = (max == min).thenElse(0.0f,
        d/(l > 0.5f).thenElse(2.0f - max - min, max + min));
    SkNf h = (max != r).thenElse(0.0f,
        (g - b)*d_inv + (g < b).thenElse(6.0f, 0.0f));
    h = (max == g).thenElse((b - r)*d_inv + 2.0f, h);
    h = (max == b).thenElse((r - g)*d_inv + 4.0f, h);
    h *= (1/6.0f);

    h = (max == min).thenElse(0.0f, h);

    r = h;
    g = s;
    b = l;
}

STAGE(hsl_to_rgb) {
    auto h = r;
    auto s = g;
    auto l = b;
    auto q = (l < 0.5f).thenElse(l*(1.0f + s), l + s - l*s);
    auto p = 2.0f*l - q;

    auto hue_to_rgb = [](const SkNf& p, const SkNf& q, const SkNf& t) {
        auto t2 = (t < 0.0f).thenElse(t + 1.0f, (t > 1.0f).thenElse(t - 1.0f, t));
        return (t2 < (1/6.0f)).thenElse(
            p + (q - p)*6.0f*t, (t2 < (3/6.0f)).thenElse(
                q, (t2 < (4/6.0f)).thenElse(
                    p + (q - p)*((4/6.0f) - t2)*6.0f, p)));
    };

    r = (s == 0.f).thenElse(l, hue_to_rgb(p, q, h + (1/3.0f)));
    g = (s == 0.f).thenElse(l, hue_to_rgb(p, q, h));
    b = (s == 0.f).thenElse(l, hue_to_rgb(p, q, h - (1/3.0f)));
}

STAGE_CTX(matrix_2x3, const float*) {
    auto m = ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[2], m[4])),
         G = SkNf_fma(r,m[1], SkNf_fma(g,m[3], m[5]));
    r = R;
    g = G;
}
STAGE_CTX(matrix_3x4, const float*) {
    auto m = ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[3], SkNf_fma(b,m[6], m[ 9]))),
         G = SkNf_fma(r,m[1], SkNf_fma(g,m[4], SkNf_fma(b,m[7], m[10]))),
         B = SkNf_fma(r,m[2], SkNf_fma(g,m[5], SkNf_fma(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}
STAGE_CTX(matrix_4x5, const float*) {
    auto m = ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[4], SkNf_fma(b,m[ 8], SkNf_fma(a,m[12], m[16])))),
         G = SkNf_fma(r,m[1], SkNf_fma(g,m[5], SkNf_fma(b,m[ 9], SkNf_fma(a,m[13], m[17])))),
         B = SkNf_fma(r,m[2], SkNf_fma(g,m[6], SkNf_fma(b,m[10], SkNf_fma(a,m[14], m[18])))),
         A = SkNf_fma(r,m[3], SkNf_fma(g,m[7], SkNf_fma(b,m[11], SkNf_fma(a,m[15], m[19]))));
    r = R;
    g = G;
    b = B;
    a = A;
}
STAGE_CTX(matrix_perspective, const float*) {
    // N.B. unlike the matrix_NxM stages, this takes a row-major matrix.
    auto m = ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[1], m[2])),
         G = SkNf_fma(r,m[3], SkNf_fma(g,m[4], m[5])),
         Z = SkNf_fma(r,m[6], SkNf_fma(g,m[7], m[8]));
    r = R * Z.invert();
    g = G * Z.invert();
}

SI SkNf parametric(const SkNf& v, const SkColorSpaceTransferFn& p) {
    float result[N];   // Unconstrained powf() doesn't vectorize well...
    for (int i = 0; i < N; i++) {
        float s = v[i];
        result[i] = (s <= p.fD) ? p.fC * s + p.fF
                                : powf(s * p.fA + p.fB, p.fG) + p.fE;
    }
    // Clamp the output to [0, 1].
    // Max(NaN, 0) = 0, but Max(0, NaN) = NaN, so we want this exact order to ensure NaN => 0
    return SkNf::Min(SkNf::Max(SkNf::Load(result), 0.0f), 1.0f);
}
STAGE_CTX(parametric_r, const SkColorSpaceTransferFn*) { r = parametric(r, *ctx); }
STAGE_CTX(parametric_g, const SkColorSpaceTransferFn*) { g = parametric(g, *ctx); }
STAGE_CTX(parametric_b, const SkColorSpaceTransferFn*) { b = parametric(b, *ctx); }
STAGE_CTX(parametric_a, const SkColorSpaceTransferFn*) { a = parametric(a, *ctx); }

SI SkNf table(const SkNf& v, const SkTableTransferFn& table) {
    float result[N];
    for (int i = 0; i < N; i++) {
        result[i] = interp_lut(v[i], table.fData, table.fSize);
    }
    // no need to clamp - tables are by-design [0,1] -> [0,1]
    return SkNf::Load(result);
}
STAGE_CTX(table_r, const SkTableTransferFn*) { r = table(r, *ctx); }
STAGE_CTX(table_g, const SkTableTransferFn*) { g = table(g, *ctx); }
STAGE_CTX(table_b, const SkTableTransferFn*) { b = table(b, *ctx); }
STAGE_CTX(table_a, const SkTableTransferFn*) { a = table(a, *ctx); }

STAGE_CTX(color_lookup_table, const SkColorLookUpTable*) {
    const SkColorLookUpTable* colorLUT = ctx;
    SkASSERT(3 == colorLUT->inputChannels() || 4 == colorLUT->inputChannels());
    SkASSERT(3 == colorLUT->outputChannels());
    float result[3][N];
    for (int i = 0; i < N; ++i) {
        const float in[4] = { r[i], g[i], b[i], a[i] };
        float out[3];
        colorLUT->interp(out, in);
        for (int j = 0; j < colorLUT->outputChannels(); ++j) {
            result[j][i] = out[j];
        }
    }
    r = SkNf::Load(result[0]);
    g = SkNf::Load(result[1]);
    b = SkNf::Load(result[2]);
    if (4 == colorLUT->inputChannels()) {
        // we must set the pixel to opaque, as the alpha channel was used
        // as input before this.
        a = 1.f;
    }
}

STAGE(lab_to_xyz) {
    const auto lab_l = r * 100.0f;
    const auto lab_a = g * 255.0f - 128.0f;
    const auto lab_b = b * 255.0f - 128.0f;
    auto Y = (lab_l + 16.0f) * (1/116.0f);
    auto X = lab_a * (1/500.0f) + Y;
    auto Z = Y - (lab_b * (1/200.0f));

    const auto X3 = X*X*X;
    X = (X3 > 0.008856f).thenElse(X3, (X - (16/116.0f)) * (1/7.787f));
    const auto Y3 = Y*Y*Y;
    Y = (Y3 > 0.008856f).thenElse(Y3, (Y - (16/116.0f)) * (1/7.787f));
    const auto Z3 = Z*Z*Z;
    Z = (Z3 > 0.008856f).thenElse(Z3, (Z - (16/116.0f)) * (1/7.787f));

    // adjust to D50 illuminant
    X *= 0.96422f;
    Y *= 1.00000f;
    Z *= 0.82521f;

    r = X;
    g = Y;
    b = Z;
}

SI SkNf assert_in_tile(const SkNf& v, float limit) {
    for (int i = 0; i < N; i++) {
        SkASSERT(0 <= v[i] && v[i] < limit);
    }
    return v;
}

SI SkNf ulp_before(float v) {
    SkASSERT(v > 0);
    SkNf vs(v);
    SkNu uvs = SkNu::Load(&vs) - 1;
    return SkNf::Load(&uvs);
}

SI SkNf clamp(const SkNf& v, float limit) {
    SkNf result = SkNf::Max(0, SkNf::Min(v, ulp_before(limit)));
    return assert_in_tile(result, limit);
}
SI SkNf repeat(const SkNf& v, float limit) {
    SkNf result = v - (v/limit).floor()*limit;
    // For small negative v, (v/limit).floor()*limit can dominate v in the subtraction,
    // which leaves result == limit.  We want result < limit, so clamp it one ULP.
    result = SkNf::Min(result, ulp_before(limit));
    return assert_in_tile(result, limit);
}
SI SkNf mirror(const SkNf& v, float l/*imit*/) {
    SkNf result = ((v - l) - ((v - l) / (2*l)).floor()*(2*l) - l).abs();
    // Same deal as repeat.
    result = SkNf::Min(result, ulp_before(l));
    return assert_in_tile(result, l);
}
STAGE_CTX( clamp_x, const float*) { r = clamp (r, *ctx); }
STAGE_CTX(repeat_x, const float*) { r = repeat(r, *ctx); }
STAGE_CTX(mirror_x, const float*) { r = mirror(r, *ctx); }
STAGE_CTX( clamp_y, const float*) { g = clamp (g, *ctx); }
STAGE_CTX(repeat_y, const float*) { g = repeat(g, *ctx); }
STAGE_CTX(mirror_y, const float*) { g = mirror(g, *ctx); }

STAGE_CTX(save_xy, SkJumper_SamplerCtx*) {
    r.store(ctx->x);
    g.store(ctx->y);

    // Whether bilinear or bicubic, all sample points have the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y), all (0.5,0.5) off-center.
    auto fract = [](const SkNf& v) { return v - v.floor(); };
    fract(r + 0.5f).store(ctx->fx);
    fract(g + 0.5f).store(ctx->fy);
}

STAGE_CTX(accumulate, const SkJumper_SamplerCtx*) {
    // Bilinear and bicubic filtering are both separable, so we'll end up with independent
    // scale contributions in x and y that we multiply together to get each pixel's scale factor.
    auto scale = SkNf::Load(ctx->scalex) * SkNf::Load(ctx->scaley);
    dr = SkNf_fma(scale, r, dr);
    dg = SkNf_fma(scale, g, dg);
    db = SkNf_fma(scale, b, db);
    da = SkNf_fma(scale, a, da);
}

// In bilinear interpolation, the 4 pixels at +/- 0.5 offsets from the sample pixel center
// are combined in direct proportion to their area overlapping that logical query pixel.
// At positive offsets, the x-axis contribution to that rectangular area is fx; (1-fx)
// at negative x offsets.  The y-axis is treated symmetrically.
template <int Scale>
SI void bilinear_x(SkJumper_SamplerCtx* ctx, SkNf* x) {
    *x = SkNf::Load(ctx->x) + Scale*0.5f;
    auto fx = SkNf::Load(ctx->fx);
    (Scale > 0 ? fx : (1.0f - fx)).store(ctx->scalex);
}
template <int Scale>
SI void bilinear_y(SkJumper_SamplerCtx* ctx, SkNf* y) {
    *y = SkNf::Load(ctx->y) + Scale*0.5f;
    auto fy = SkNf::Load(ctx->fy);
    (Scale > 0 ? fy : (1.0f - fy)).store(ctx->scaley);
}
STAGE_CTX(bilinear_nx, SkJumper_SamplerCtx*) { bilinear_x<-1>(ctx, &r); }
STAGE_CTX(bilinear_px, SkJumper_SamplerCtx*) { bilinear_x<+1>(ctx, &r); }
STAGE_CTX(bilinear_ny, SkJumper_SamplerCtx*) { bilinear_y<-1>(ctx, &g); }
STAGE_CTX(bilinear_py, SkJumper_SamplerCtx*) { bilinear_y<+1>(ctx, &g); }


// In bilinear interpolation, the 16 pixels at +/- 0.5 and +/- 1.5 offsets from the sample
// pixel center are combined with a non-uniform cubic filter, with high filter values near
// the center and lower values farther away.
//
// We break this filter function into two parts, one for near +/- 0.5 offsets,
// and one for far +/- 1.5 offsets.
//
// See GrBicubicEffect for details about this particular Mitchell-Netravali filter.
SI SkNf bicubic_near(const SkNf& t) {
    // 1/18 + 9/18t + 27/18t^2 - 21/18t^3 == t ( t ( -21/18t + 27/18) + 9/18) + 1/18
    return SkNf_fma(t, SkNf_fma(t, SkNf_fma(-21/18.0f, t, 27/18.0f), 9/18.0f), 1/18.0f);
}
SI SkNf bicubic_far(const SkNf& t) {
    // 0/18 + 0/18*t - 6/18t^2 + 7/18t^3 == t^2 (7/18t - 6/18)
    return (t*t)*SkNf_fma(7/18.0f, t, -6/18.0f);
}

template <int Scale>
SI void bicubic_x(SkJumper_SamplerCtx* ctx, SkNf* x) {
    *x = SkNf::Load(ctx->x) + Scale*0.5f;
    auto fx = SkNf::Load(ctx->fx);
    if (Scale == -3) { return bicubic_far (1.0f - fx).store(ctx->scalex); }
    if (Scale == -1) { return bicubic_near(1.0f - fx).store(ctx->scalex); }
    if (Scale == +1) { return bicubic_near(       fx).store(ctx->scalex); }
    if (Scale == +3) { return bicubic_far (       fx).store(ctx->scalex); }
    SkDEBUGFAIL("unreachable");
}
template <int Scale>
SI void bicubic_y(SkJumper_SamplerCtx* ctx, SkNf* y) {
    *y = SkNf::Load(ctx->y) + Scale*0.5f;
    auto fy = SkNf::Load(ctx->fy);
    if (Scale == -3) { return bicubic_far (1.0f - fy).store(ctx->scaley); }
    if (Scale == -1) { return bicubic_near(1.0f - fy).store(ctx->scaley); }
    if (Scale == +1) { return bicubic_near(       fy).store(ctx->scaley); }
    if (Scale == +3) { return bicubic_far (       fy).store(ctx->scaley); }
    SkDEBUGFAIL("unreachable");
}
STAGE_CTX(bicubic_n3x, SkJumper_SamplerCtx*) { bicubic_x<-3>(ctx, &r); }
STAGE_CTX(bicubic_n1x, SkJumper_SamplerCtx*) { bicubic_x<-1>(ctx, &r); }
STAGE_CTX(bicubic_p1x, SkJumper_SamplerCtx*) { bicubic_x<+1>(ctx, &r); }
STAGE_CTX(bicubic_p3x, SkJumper_SamplerCtx*) { bicubic_x<+3>(ctx, &r); }

STAGE_CTX(bicubic_n3y, SkJumper_SamplerCtx*) { bicubic_y<-3>(ctx, &g); }
STAGE_CTX(bicubic_n1y, SkJumper_SamplerCtx*) { bicubic_y<-1>(ctx, &g); }
STAGE_CTX(bicubic_p1y, SkJumper_SamplerCtx*) { bicubic_y<+1>(ctx, &g); }
STAGE_CTX(bicubic_p3y, SkJumper_SamplerCtx*) { bicubic_y<+3>(ctx, &g); }


template <typename T>
SI SkNi offset_and_ptr(T** ptr, const SkJumper_GatherCtx* ctx, const SkNf& x, const SkNf& y) {
    SkNi ix = SkNx_cast<int>(x),
         iy = SkNx_cast<int>(y);
    SkNi offset = iy*ctx->stride + ix;

    *ptr = (const T*)ctx->pixels;
    return offset;
}

STAGE_CTX(gather_a8, const SkJumper_GatherCtx*) {
    const uint8_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    r = g = b = 0.0f;
    a = SkNf_from_byte(gather(tail, p, offset));
}
STAGE_CTX(gather_i8, const SkJumper_GatherCtx*) {
    const uint8_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    SkNi ix = SkNx_cast<int>(gather(tail, p, offset));
    from_8888(gather(tail, ctx->ctable, ix), &r, &g, &b, &a);
}
STAGE_CTX(gather_g8, const SkJumper_GatherCtx*) {
    const uint8_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    r = g = b = SkNf_from_byte(gather(tail, p, offset));
    a = 1.0f;
}
STAGE_CTX(gather_565, const SkJumper_GatherCtx*) {
    const uint16_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    from_565(gather(tail, p, offset), &r, &g, &b);
    a = 1.0f;
}
STAGE_CTX(gather_4444, const SkJumper_GatherCtx*) {
    const uint16_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    from_4444(gather(tail, p, offset), &r, &g, &b, &a);
}
STAGE_CTX(gather_8888, const SkJumper_GatherCtx*) {
    const uint32_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    from_8888(gather(tail, p, offset), &r, &g, &b, &a);
}
STAGE_CTX(gather_f16, const SkJumper_GatherCtx*) {
    const uint64_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    auto px = gather(tail, p, offset);
    from_f16(&px, &r, &g, &b, &a);
}

STAGE_CTX(linear_gradient, const SkPM4f*) {
    struct Stop { float pos; float f[4], b[4]; };
    struct Ctx { size_t n; Stop *stops; float start[4]; };

    auto c = (const Ctx*)ctx;
    SkNf fr = 0, fg = 0, fb = 0, fa = 0;
    SkNf br = c->start[0],
         bg = c->start[1],
         bb = c->start[2],
         ba = c->start[3];
    auto t = r;
    for (size_t i = 0; i < c->n; i++) {
        fr = (t < c->stops[i].pos).thenElse(fr, c->stops[i].f[0]);
        fg = (t < c->stops[i].pos).thenElse(fg, c->stops[i].f[1]);
        fb = (t < c->stops[i].pos).thenElse(fb, c->stops[i].f[2]);
        fa = (t < c->stops[i].pos).thenElse(fa, c->stops[i].f[3]);
        br = (t < c->stops[i].pos).thenElse(br, c->stops[i].b[0]);
        bg = (t < c->stops[i].pos).thenElse(bg, c->stops[i].b[1]);
        bb = (t < c->stops[i].pos).thenElse(bb, c->stops[i].b[2]);
        ba = (t < c->stops[i].pos).thenElse(ba, c->stops[i].b[3]);
    }

    r = SkNf_fma(t, fr, br);
    g = SkNf_fma(t, fg, bg);
    b = SkNf_fma(t, fb, bb);
    a = SkNf_fma(t, fa, ba);
}

STAGE_CTX(linear_gradient_2stops, const SkPM4f*) {
    auto t = r;
    SkPM4f c0 = ctx[0],
        dc = ctx[1];

    r = SkNf_fma(t, dc.r(), c0.r());
    g = SkNf_fma(t, dc.g(), c0.g());
    b = SkNf_fma(t, dc.b(), c0.b());
    a = SkNf_fma(t, dc.a(), c0.a());
}

STAGE_CTX(byte_tables, const void*) {
    struct Tables { const uint8_t *r, *g, *b, *a; };
    auto tables = (const Tables*)ctx;

    r = SkNf_from_byte(gather(tail, tables->r, SkNf_round(255.0f, r)));
    g = SkNf_from_byte(gather(tail, tables->g, SkNf_round(255.0f, g)));
    b = SkNf_from_byte(gather(tail, tables->b, SkNf_round(255.0f, b)));
    a = SkNf_from_byte(gather(tail, tables->a, SkNf_round(255.0f, a)));
}

STAGE_CTX(byte_tables_rgb, const void*) {
    struct Tables { const uint8_t *r, *g, *b; int n; };
    auto tables = (const Tables*)ctx;

    float scale = tables->n - 1;
    r = SkNf_from_byte(gather(tail, tables->r, SkNf_round(scale, r)));
    g = SkNf_from_byte(gather(tail, tables->g, SkNf_round(scale, g)));
    b = SkNf_from_byte(gather(tail, tables->b, SkNf_round(scale, b)));
}

STAGE_CTX(shader_adapter, SkShader::Context*) {
    SkPM4f buf[N];
    static_assert(sizeof(buf) == sizeof(r) + sizeof(g) + sizeof(b) + sizeof(a), "");
    ctx->shadeSpan4f(x, (int)g[0], buf, N);
    SkNf::Load4(buf, &r, &g, &b, &a);
}

SI Fn enum_to_Fn(SkRasterPipeline::StockStage st) {
    switch (st) {
    #define M(stage) case SkRasterPipeline::stage: return stage;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
    }
    SkASSERT(false);
    return just_return;
}

namespace {

    static void build_program(void** program, const SkRasterPipeline::Stage* stages, int nstages) {
        for (int i = 0; i < nstages; i++) {
            *program++ = (void*)enum_to_Fn(stages[i].stage);
            if (stages[i].ctx) {
                *program++ = stages[i].ctx;
            }
        }
        *program++ = (void*)just_return;
    }

    static void run_program(void** program, size_t x, size_t n) {
        SkNf u;  // fastest to start uninitialized.

        auto start = (Fn)load_and_increment(&program);
        while (n >= N) {
            start(x*N, program, u,u,u,u, u,u,u,u);
            x += N;
            n -= N;
        }
        if (n) {
            start(x*N+n, program, u,u,u,u, u,u,u,u);
        }
    }

    // Compiled manages its memory manually because it's not safe to use
    // std::vector, SkTDArray, etc without setting us up for big ODR violations.
    struct Compiled {
        Compiled(const SkRasterPipeline::Stage* stages, int nstages) {
            int slots = nstages + 1;  // One extra for just_return.
            for (int i = 0; i < nstages; i++) {
                if (stages[i].ctx) {
                    slots++;
                }
            }
            fProgram = (void**)sk_malloc_throw(slots * sizeof(void*));
            build_program(fProgram, stages, nstages);
        }
        ~Compiled() { sk_free(fProgram); }

        Compiled(const Compiled& o) {
            int slots = 0;
            while (o.fProgram[slots++] != (void*)just_return);

            fProgram = (void**)sk_malloc_throw(slots * sizeof(void*));
            memcpy(fProgram, o.fProgram, slots * sizeof(void*));
        }

        void operator()(size_t x, size_t n) {
            run_program(fProgram, x, n);
        }

        void** fProgram;
    };
}

namespace SK_OPTS_NS {

    SI void run_pipeline(size_t x, size_t n,
                         const SkRasterPipeline::Stage* stages, int nstages) {
        static const int kStackMax = 256;
        // Worst case is nstages stages with nstages context pointers, and just_return.
        if (2*nstages+1 <= kStackMax) {
            void* program[kStackMax];
            build_program(program, stages, nstages);
            run_program(program, x,n);
        } else {
            Compiled{stages,nstages}(x,n);
        }
    }

}  // namespace SK_OPTS_NS

#undef SI
#undef STAGE
#undef STAGE_CTX
#undef RGBA_XFERMODE
#undef RGB_XFERMODE

#endif//SkRasterPipeline_opts_DEFINED
