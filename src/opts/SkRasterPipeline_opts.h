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
#include "SkImageShaderContext.h"
#include "SkMSAN.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"
#include "SkRasterPipeline.h"
#include "SkSRGB.h"
#include <utility>

namespace {

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    static constexpr int N = 8;
#else
    static constexpr int N = 4;
#endif

    using SkNf = SkNx<N, float>;
    using SkNi = SkNx<N, int32_t>;
    using SkNu = SkNx<N, uint32_t>;
    using SkNh = SkNx<N, uint16_t>;
    using SkNb = SkNx<N, uint8_t>;

    struct Stage;
    using Fn = void(SK_VECTORCALL *)(Stage*, size_t x_tail, SkNf,SkNf,SkNf,SkNf,
                                                            SkNf,SkNf,SkNf,SkNf);
    struct Stage { Fn next; void* ctx; };

    // x_tail encodes two values x and tail as x*N+tail, where 0 <= tail < N.
    // x is the induction variable we're walking along, incrementing by N each step.
    // tail == 0 means work with a full N pixels; otherwise use only the low tail pixels.

}  // namespace

#define SI static inline

// Stages are logically a pipeline, and physically are contiguous in an array.
// To get to the next stage, we just increment our pointer to the next array element.
SI void SK_VECTORCALL next(Stage* st, size_t x_tail, SkNf  r, SkNf  g, SkNf  b, SkNf  a,
                                                     SkNf dr, SkNf dg, SkNf db, SkNf da) {
    st->next(st+1, x_tail, r,g,b,a, dr,dg,db,da);
}

// Stages defined below always call next.
// This is always the last stage, a backstop that actually returns to the caller when done.
SI void SK_VECTORCALL just_return(Stage*, size_t, SkNf, SkNf, SkNf, SkNf,
                                                  SkNf, SkNf, SkNf, SkNf) {}

#define STAGE(name)                                                                      \
    static SK_ALWAYS_INLINE void name##_kernel(void* ctx, size_t x, size_t tail,         \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,   \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da);  \
    SI void SK_VECTORCALL name(Stage* st, size_t x_tail,                                 \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,                       \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {                     \
        name##_kernel(st->ctx, x_tail/N, x_tail%N, r,g,b,a, dr,dg,db,da);                \
        next(st, x_tail, r,g,b,a, dr,dg,db,da);                                          \
    }                                                                                    \
    static SK_ALWAYS_INLINE void name##_kernel(void* ctx, size_t x, size_t tail,         \
                                               SkNf&  r, SkNf&  g, SkNf&  b, SkNf&  a,   \
                                               SkNf& dr, SkNf& dg, SkNf& db, SkNf& da)


// Many xfermodes apply the same logic to each channel.
#define RGBA_XFERMODE(name)                                                     \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da);  \
    SI void SK_VECTORCALL name(Stage* st, size_t x_tail,                        \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,              \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {            \
        r = name##_kernel(r,a,dr,da);                                           \
        g = name##_kernel(g,a,dg,da);                                           \
        b = name##_kernel(b,a,db,da);                                           \
        a = name##_kernel(a,a,da,da);                                           \
        next(st, x_tail, r,g,b,a, dr,dg,db,da);                                 \
    }                                                                           \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da)

// Most of the rest apply the same logic to color channels and use srcover's alpha logic.
#define RGB_XFERMODE(name)                                                      \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da);  \
    SI void SK_VECTORCALL name(Stage* st, size_t x_tail,                         \
                               SkNf  r, SkNf  g, SkNf  b, SkNf  a,              \
                               SkNf dr, SkNf dg, SkNf db, SkNf da) {            \
        r = name##_kernel(r,a,dr,da);                                           \
        g = name##_kernel(g,a,dg,da);                                           \
        b = name##_kernel(b,a,db,da);                                           \
        a = a + (da * (1.0f-a));                                                \
        next(st, x_tail, r,g,b,a, dr,dg,db,da);                                 \
    }                                                                           \
    static SK_ALWAYS_INLINE SkNf name##_kernel(const SkNf& s, const SkNf& sa,   \
                                               const SkNf& d, const SkNf& da)

template <typename T>
SI SkNx<N,T> load(size_t tail, const T* src) {
    if (tail) {
        T buf[8] = {0};
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

#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    SI __m256i mask(size_t tail) {
        static const int masks[][8] = {
            {~0,~0,~0,~0, ~0,~0,~0,~0 },  // remember, tail == 0 ~~> load all N
            {~0, 0, 0, 0,  0, 0, 0, 0 },
            {~0,~0, 0, 0,  0, 0, 0, 0 },
            {~0,~0,~0, 0,  0, 0, 0, 0 },
            {~0,~0,~0,~0,  0, 0, 0, 0 },
            {~0,~0,~0,~0, ~0, 0, 0, 0 },
            {~0,~0,~0,~0, ~0,~0, 0, 0 },
            {~0,~0,~0,~0, ~0,~0,~0, 0 },
        };
        return SkNi::Load(masks + tail).fVec;
    }

    SI SkNi load(size_t tail, const  int32_t* src) {
        return tail ? _mm256_maskload_epi32((const int*)src, mask(tail))
                    : SkNi::Load(src);
    }
    SI SkNu load(size_t tail, const uint32_t* src) {
        return tail ? _mm256_maskload_epi32((const int*)src, mask(tail))
                    : SkNu::Load(src);
    }
    SI SkNf load(size_t tail, const float* src) {
        return tail ? _mm256_maskload_ps((const float*)src, mask(tail))
                    : SkNf::Load(src);
    }
    SI SkNi gather(size_t tail, const  int32_t* src, const SkNi& offset) {
        auto m = mask(tail);
        return _mm256_mask_i32gather_epi32(SkNi(0).fVec, (const int*)src, offset.fVec, m, 4);
    }
    SI SkNu gather(size_t tail, const uint32_t* src, const SkNi& offset) {
        auto m = mask(tail);
        return _mm256_mask_i32gather_epi32(SkNi(0).fVec, (const int*)src, offset.fVec, m, 4);
    }
    SI SkNf gather(size_t tail, const float* src, const SkNi& offset) {
        auto m = _mm256_castsi256_ps(mask(tail));
        return _mm256_mask_i32gather_ps(SkNf(0).fVec, (const float*)src, offset.fVec, m, 4);
    }

    static const char* bug = "I don't think MSAN understands maskstore.";

    SI void store(size_t tail, const SkNi& v,  int32_t* dst) {
        if (tail) {
            _mm256_maskstore_epi32((int*)dst, mask(tail), v.fVec);
            return sk_msan_mark_initialized(dst, dst+tail, bug);
        }
        v.store(dst);
    }
    SI void store(size_t tail, const SkNu& v, uint32_t* dst) {
        if (tail) {
            _mm256_maskstore_epi32((int*)dst, mask(tail), v.fVec);
            return sk_msan_mark_initialized(dst, dst+tail, bug);
        }
        v.store(dst);
    }
    SI void store(size_t tail, const SkNf& v, float* dst) {
        if (tail) {
            _mm256_maskstore_ps((float*)dst, mask(tail), v.fVec);
            return sk_msan_mark_initialized(dst, dst+tail, bug);
        }
        v.store(dst);
    }
#endif

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

STAGE(trace) {
    SkDebugf("%s\n", (const char*)ctx);
}
STAGE(registers) {
    auto print = [](const char* name, const SkNf& v) {
        SkDebugf("%s:", name);
        for (int i = 0; i < N; i++) {
            SkDebugf(" %g", v[i]);
        }
        SkDebugf("\n");
    };
    print(" r",  r);
    print(" g",  g);
    print(" b",  b);
    print(" a",  a);
    print("dr", dr);
    print("dg", dg);
    print("db", db);
    print("da", da);
}

STAGE(clamp_0) {
    a = SkNf::Max(a, 0.0f);
    r = SkNf::Max(r, 0.0f);
    g = SkNf::Max(g, 0.0f);
    b = SkNf::Max(b, 0.0f);
}
STAGE(clamp_a) {
    a = SkNf::Min(a, 1.0f);
    r = SkNf::Min(r, a);
    g = SkNf::Min(g, a);
    b = SkNf::Min(b, a);
}
STAGE(clamp_1) {
    a = SkNf::Min(a, 1.0f);
    r = SkNf::Min(r, 1.0f);
    g = SkNf::Min(g, 1.0f);
    b = SkNf::Min(b, 1.0f);
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

STAGE(set_rgb) {
    auto rgb = (const float*)ctx;
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}

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

STAGE(swap_rb)   { SkTSwap( r,  b); }
STAGE(swap_rb_d) { SkTSwap(dr, db); }

STAGE(from_srgb) {
    r = sk_linear_from_srgb_math(r);
    g = sk_linear_from_srgb_math(g);
    b = sk_linear_from_srgb_math(b);
}
STAGE(from_srgb_d) {
    dr = sk_linear_from_srgb_math(dr);
    dg = sk_linear_from_srgb_math(dg);
    db = sk_linear_from_srgb_math(db);
}
STAGE(to_srgb) {
    r = sk_linear_to_srgb_needs_round(r);
    g = sk_linear_to_srgb_needs_round(g);
    b = sk_linear_to_srgb_needs_round(b);
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
STAGE(constant_color) {
    auto color = (const SkPM4f*)ctx;
    r = color->r();
    g = color->g();
    b = color->b();
    a = color->a();
}

// s' = sc for a scalar c.
STAGE(scale_1_float) {
    SkNf c = *(const float*)ctx;

    r *= c;
    g *= c;
    b *= c;
    a *= c;
}
// s' = sc for 8-bit c.
STAGE(scale_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

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
STAGE(lerp_1_float) {
    SkNf c = *(const float*)ctx;

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// s' = d(1-c) + sc for 8-bit c.
STAGE(lerp_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    SkNf c = SkNf_from_byte(load(tail, ptr));
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// s' = d(1-c) + sc for 565 c.
STAGE(lerp_565) {
    auto ptr = *(const uint16_t**)ctx + x;
    SkNf cr, cg, cb;
    from_565(load(tail, ptr), &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0f;
}

STAGE(load_565) {
    auto ptr = *(const uint16_t**)ctx + x;
    from_565(load(tail, ptr), &r,&g,&b);
    a = 1.0f;
}
STAGE(load_565_d) {
    auto ptr = *(const uint16_t**)ctx + x;
    from_565(load(tail, ptr), &dr,&dg,&db);
    da = 1.0f;
}
STAGE(store_565) {
    auto ptr = *(uint16_t**)ctx + x;
    store(tail, SkNx_cast<uint16_t>( SkNf_round(r, SK_R16_MASK) << SK_R16_SHIFT
                                   | SkNf_round(g, SK_G16_MASK) << SK_G16_SHIFT
                                   | SkNf_round(b, SK_B16_MASK) << SK_B16_SHIFT), ptr);
}


STAGE(load_f16) {
    auto ptr = *(const uint64_t**)ctx + x;

    SkNh rh, gh, bh, ah;
    if (tail) {
        uint64_t buf[8] = {0};
        switch (tail & (N-1)) {
            case 7: buf[6] = ptr[6];
            case 6: buf[5] = ptr[5];
            case 5: buf[4] = ptr[4];
            case 4: buf[3] = ptr[3];
            case 3: buf[2] = ptr[2];
            case 2: buf[1] = ptr[1];
        }
        buf[0] = ptr[0];
        SkNh::Load4(buf, &rh, &gh, &bh, &ah);
    } else {
        SkNh::Load4(ptr, &rh, &gh, &bh, &ah);
    }

    r = SkHalfToFloat_finite_ftz(rh);
    g = SkHalfToFloat_finite_ftz(gh);
    b = SkHalfToFloat_finite_ftz(bh);
    a = SkHalfToFloat_finite_ftz(ah);
}
STAGE(load_f16_d) {
    auto ptr = *(const uint64_t**)ctx + x;

    SkNh rh, gh, bh, ah;
    if (tail) {
        uint64_t buf[8] = {0};
        switch (tail & (N-1)) {
            case 7: buf[6] = ptr[6];
            case 6: buf[5] = ptr[5];
            case 5: buf[4] = ptr[4];
            case 4: buf[3] = ptr[3];
            case 3: buf[2] = ptr[2];
            case 2: buf[1] = ptr[1];
        }
        buf[0] = ptr[0];
        SkNh::Load4(buf, &rh, &gh, &bh, &ah);
    } else {
        SkNh::Load4(ptr, &rh, &gh, &bh, &ah);
    }

    dr = SkHalfToFloat_finite_ftz(rh);
    dg = SkHalfToFloat_finite_ftz(gh);
    db = SkHalfToFloat_finite_ftz(bh);
    da = SkHalfToFloat_finite_ftz(ah);
}
STAGE(store_f16) {
    auto ptr = *(uint64_t**)ctx + x;

    uint64_t buf[8];
    SkNh::Store4(tail ? buf : ptr, SkFloatToHalf_finite_ftz(r),
                                   SkFloatToHalf_finite_ftz(g),
                                   SkFloatToHalf_finite_ftz(b),
                                   SkFloatToHalf_finite_ftz(a));
    if (tail) {
        switch (tail & (N-1)) {
            case 7: ptr[6] = buf[6];
            case 6: ptr[5] = buf[5];
            case 5: ptr[4] = buf[4];
            case 4: ptr[3] = buf[3];
            case 3: ptr[2] = buf[2];
            case 2: ptr[1] = buf[1];
        }
        ptr[0] = buf[0];
    }
}

STAGE(store_f32) {
    auto ptr = *(SkPM4f**)ctx + x;

    SkPM4f buf[8];
    SkNf::Store4(tail ? buf : ptr, r,g,b,a);
    if (tail) {
        switch (tail & (N-1)) {
            case 7: ptr[6] = buf[6];
            case 6: ptr[5] = buf[5];
            case 5: ptr[4] = buf[4];
            case 4: ptr[3] = buf[3];
            case 3: ptr[2] = buf[2];
            case 2: ptr[1] = buf[1];
        }
        ptr[0] = buf[0];
    }
}


STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load(tail, ptr), &r, &g, &b, &a);
}
STAGE(load_8888_d) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load(tail, ptr), &dr, &dg, &db, &da);
}
STAGE(store_8888) {
    auto byte = [](const SkNf& x, int ix) {
        // Here's a neat trick: 0x47000000 == 32768.0f, and 0x470000ff == 32768.0f + (255/256.0f).
        auto v = SkNf_fma(255/256.0f, x, 32768.0f);
        switch (ix) {
            case 0: return SkNi::Load(&v) & 0xff;  // R
            case 3: return SkNi::Load(&v) << 24;   // A
        }
        return (SkNi::Load(&v) & 0xff) << (8*ix);  // B or G
    };

    auto ptr = *(uint32_t**)ctx + x;
    store(tail, byte(r,0)|byte(g,1)|byte(b,2)|byte(a,3), (int*)ptr);
}

STAGE(load_tables) {
    auto loadCtx = (const LoadTablesContext*)ctx;
    auto ptr = loadCtx->fSrc + x;

    SkNu rgba = load(tail, ptr);
    auto to_int = [](const SkNu& v) { return SkNi::Load(&v); };
    r = gather(tail, loadCtx->fR, to_int((rgba >>  0) & 0xff));
    g = gather(tail, loadCtx->fG, to_int((rgba >>  8) & 0xff));
    b = gather(tail, loadCtx->fB, to_int((rgba >> 16) & 0xff));
    a = SkNf_from_byte(rgba >> 24);
}

STAGE(store_tables) {
    auto storeCtx = (const StoreTablesContext*)ctx;
    auto ptr = storeCtx->fDst + x;

    float scale = storeCtx->fCount - 1;
    SkNi ri = SkNf_round(scale, r);
    SkNi gi = SkNf_round(scale, g);
    SkNi bi = SkNf_round(scale, b);

    store(tail, ( SkNx_cast<int>(gather(tail, storeCtx->fR, ri)) << 0
                | SkNx_cast<int>(gather(tail, storeCtx->fG, gi)) << 8
                | SkNx_cast<int>(gather(tail, storeCtx->fB, bi)) << 16
                | SkNf_round(255.0f, a)                          << 24), (int*)ptr);
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

STAGE(matrix_2x3) {
    auto m = (const float*)ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[2], m[4])),
         G = SkNf_fma(r,m[1], SkNf_fma(g,m[3], m[5]));
    r = R;
    g = G;
}
STAGE(matrix_3x4) {
    auto m = (const float*)ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[3], SkNf_fma(b,m[6], m[ 9]))),
         G = SkNf_fma(r,m[1], SkNf_fma(g,m[4], SkNf_fma(b,m[7], m[10]))),
         B = SkNf_fma(r,m[2], SkNf_fma(g,m[5], SkNf_fma(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}
STAGE(matrix_4x5) {
    auto m = (const float*)ctx;

    auto R = SkNf_fma(r,m[0], SkNf_fma(g,m[4], SkNf_fma(b,m[ 8], SkNf_fma(a,m[12], m[16])))),
         G = SkNf_fma(r,m[1], SkNf_fma(g,m[5], SkNf_fma(b,m[ 9], SkNf_fma(a,m[13], m[17])))),
         B = SkNf_fma(r,m[2], SkNf_fma(g,m[6], SkNf_fma(b,m[10], SkNf_fma(a,m[14], m[18])))),
         A = SkNf_fma(r,m[3], SkNf_fma(g,m[7], SkNf_fma(b,m[11], SkNf_fma(a,m[15], m[19]))));
    r = R;
    g = G;
    b = B;
    a = A;
}
STAGE(matrix_perspective) {
    // N.B. unlike the matrix_NxM stages, this takes a row-major matrix.
    auto m = (const float*)ctx;

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
        result[i] = (s <= p.fD) ? p.fE * s + p.fF
                                : powf(s * p.fA + p.fB, p.fG) + p.fC;
    }
    // Clamp the output to [0, 1].
    // Max(NaN, 0) = 0, but Max(0, NaN) = NaN, so we want this exact order to ensure NaN => 0
    return SkNf::Min(SkNf::Max(SkNf::Load(result), 0.0f), 1.0f);
}
STAGE(parametric_r) { r = parametric(r, *(const SkColorSpaceTransferFn*)ctx); }
STAGE(parametric_g) { g = parametric(g, *(const SkColorSpaceTransferFn*)ctx); }
STAGE(parametric_b) { b = parametric(b, *(const SkColorSpaceTransferFn*)ctx); }
STAGE(parametric_a) { a = parametric(a, *(const SkColorSpaceTransferFn*)ctx); }

SI SkNf table(const SkNf& v, const SkTableTransferFn& table) {
    float result[N];
    for (int i = 0; i < N; i++) {
        result[i] = interp_lut(v[i], table.fData, table.fSize);
    }
    // no need to clamp - tables are by-design [0,1] -> [0,1]
    return SkNf::Load(result);
}
STAGE(table_r) { r = table(r, *(const SkTableTransferFn*)ctx); }
STAGE(table_g) { g = table(g, *(const SkTableTransferFn*)ctx); }
STAGE(table_b) { b = table(b, *(const SkTableTransferFn*)ctx); }
STAGE(table_a) { a = table(a, *(const SkTableTransferFn*)ctx); }

STAGE(color_lookup_table) {
    const SkColorLookUpTable* colorLUT = (const SkColorLookUpTable*)ctx;
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

SI SkNf clamp(const SkNf& v, float limit) {
    SkNf result = SkNf::Max(0, SkNf::Min(v, limit - 0.5f));
    return assert_in_tile(result, limit);
}
SI SkNf repeat(const SkNf& v, float limit) {
    SkNf result = v - (v/limit).floor()*limit;
    // For small negative v, (v/limit).floor()*limit can dominate v in the subtraction,
    // which leaves result == limit.  We want result < limit, so clamp it one ULP.
    result = SkNf::Min(result, nextafterf(limit, 0));
    return assert_in_tile(result, limit);
}
SI SkNf mirror(const SkNf& v, float l/*imit*/) {
    SkNf result = ((v - l) - ((v - l) / (2*l)).floor()*(2*l) - l).abs();
    // Same deal as repeat.
    result = SkNf::Min(result, nextafterf(l, 0));
    return assert_in_tile(result, l);
}
STAGE( clamp_x) { r = clamp (r, *(const float*)ctx); }
STAGE(repeat_x) { r = repeat(r, *(const float*)ctx); }
STAGE(mirror_x) { r = mirror(r, *(const float*)ctx); }
STAGE( clamp_y) { g = clamp (g, *(const float*)ctx); }
STAGE(repeat_y) { g = repeat(g, *(const float*)ctx); }
STAGE(mirror_y) { g = mirror(g, *(const float*)ctx); }

STAGE(save_xy) {
    auto sc = (SkImageShaderContext*)ctx;

    r.store(sc->x);
    g.store(sc->y);

    // Whether bilinear or bicubic, all sample points have the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y), all (0.5,0.5) off-center.
    auto fract = [](const SkNf& v) { return v - v.floor(); };
    fract(r + 0.5f).store(sc->fx);
    fract(g + 0.5f).store(sc->fy);
}

STAGE(accumulate) {
    auto sc = (const SkImageShaderContext*)ctx;

    // Bilinear and bicubic filtering are both separable, so we'll end up with independent
    // scale contributions in x and y that we multiply together to get each pixel's scale factor.
    auto scale = SkNf::Load(sc->scalex) * SkNf::Load(sc->scaley);
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
SI void bilinear_x(void* ctx, SkNf* x) {
    auto sc = (SkImageShaderContext*)ctx;

    *x = SkNf::Load(sc->x) + Scale*0.5f;
    auto fx = SkNf::Load(sc->fx);
    (Scale > 0 ? fx : (1.0f - fx)).store(sc->scalex);
}
template <int Scale>
SI void bilinear_y(void* ctx, SkNf* y) {
    auto sc = (SkImageShaderContext*)ctx;

    *y = SkNf::Load(sc->y) + Scale*0.5f;
    auto fy = SkNf::Load(sc->fy);
    (Scale > 0 ? fy : (1.0f - fy)).store(sc->scaley);
}
STAGE(bilinear_nx) { bilinear_x<-1>(ctx, &r); }
STAGE(bilinear_px) { bilinear_x<+1>(ctx, &r); }
STAGE(bilinear_ny) { bilinear_y<-1>(ctx, &g); }
STAGE(bilinear_py) { bilinear_y<+1>(ctx, &g); }


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
SI void bicubic_x(void* ctx, SkNf* x) {
    auto sc = (SkImageShaderContext*)ctx;

    *x = SkNf::Load(sc->x) + Scale*0.5f;
    auto fx = SkNf::Load(sc->fx);
    if (Scale == -3) { return bicubic_far (1.0f - fx).store(sc->scalex); }
    if (Scale == -1) { return bicubic_near(1.0f - fx).store(sc->scalex); }
    if (Scale == +1) { return bicubic_near(       fx).store(sc->scalex); }
    if (Scale == +3) { return bicubic_far (       fx).store(sc->scalex); }
    SkDEBUGFAIL("unreachable");
}
template <int Scale>
SI void bicubic_y(void* ctx, SkNf* y) {
    auto sc = (SkImageShaderContext*)ctx;

    *y = SkNf::Load(sc->y) + Scale*0.5f;
    auto fy = SkNf::Load(sc->fy);
    if (Scale == -3) { return bicubic_far (1.0f - fy).store(sc->scaley); }
    if (Scale == -1) { return bicubic_near(1.0f - fy).store(sc->scaley); }
    if (Scale == +1) { return bicubic_near(       fy).store(sc->scaley); }
    if (Scale == +3) { return bicubic_far (       fy).store(sc->scaley); }
    SkDEBUGFAIL("unreachable");
}
STAGE(bicubic_n3x) { bicubic_x<-3>(ctx, &r); }
STAGE(bicubic_n1x) { bicubic_x<-1>(ctx, &r); }
STAGE(bicubic_p1x) { bicubic_x<+1>(ctx, &r); }
STAGE(bicubic_p3x) { bicubic_x<+3>(ctx, &r); }

STAGE(bicubic_n3y) { bicubic_y<-3>(ctx, &g); }
STAGE(bicubic_n1y) { bicubic_y<-1>(ctx, &g); }
STAGE(bicubic_p1y) { bicubic_y<+1>(ctx, &g); }
STAGE(bicubic_p3y) { bicubic_y<+3>(ctx, &g); }


template <typename T>
SI SkNi offset_and_ptr(T** ptr, const void* ctx, const SkNf& x, const SkNf& y) {
    auto sc = (const SkImageShaderContext*)ctx;

    SkNi ix = SkNx_cast<int>(x),
         iy = SkNx_cast<int>(y);
    SkNi offset = iy*sc->stride + ix;

    *ptr = (const T*)sc->pixels;
    return offset;
}

STAGE(gather_a8) {
    const uint8_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    r = g = b = 0.0f;
    a = SkNf_from_byte(gather(tail, p, offset));
}
STAGE(gather_i8) {
    auto sc = (const SkImageShaderContext*)ctx;
    const uint8_t* p;
    SkNi offset = offset_and_ptr(&p, sc, r, g);

    SkNi ix = SkNx_cast<int>(gather(tail, p, offset));
    from_8888(gather(tail, sc->ctable->readColors(), ix), &r, &g, &b, &a);
}
STAGE(gather_g8) {
    const uint8_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    r = g = b = SkNf_from_byte(gather(tail, p, offset));
    a = 1.0f;
}
STAGE(gather_565) {
    const uint16_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    from_565(gather(tail, p, offset), &r, &g, &b);
    a = 1.0f;
}
STAGE(gather_4444) {
    const uint16_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    from_4444(gather(tail, p, offset), &r, &g, &b, &a);
}
STAGE(gather_8888) {
    const uint32_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    from_8888(gather(tail, p, offset), &r, &g, &b, &a);
}
STAGE(gather_f16) {
    const uint64_t* p;
    SkNi offset = offset_and_ptr(&p, ctx, r, g);

    // f16 -> f32 conversion works best with tightly packed f16s,
    // so we gather each component rather than using gather().
    uint16_t R[N], G[N], B[N], A[N];
    size_t n = tail ? tail : N;
    for (size_t i = 0; i < n; i++) {
        uint64_t rgba = p[offset[i]];
        R[i] = rgba >>  0;
        G[i] = rgba >> 16;
        B[i] = rgba >> 32;
        A[i] = rgba >> 48;
    }
    for (size_t i = n; i < N; i++) {
        R[i] = G[i] = B[i] = A[i] = 0;
    }
    r = SkHalfToFloat_finite_ftz(SkNh::Load(R));
    g = SkHalfToFloat_finite_ftz(SkNh::Load(G));
    b = SkHalfToFloat_finite_ftz(SkNh::Load(B));
    a = SkHalfToFloat_finite_ftz(SkNh::Load(A));
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
    struct Compiled {
        Compiled(const SkRasterPipeline::Stage* stages, int nstages) : fStages(nstages) {
            if (nstages == 0) {
                return;
            }
            fStart = enum_to_Fn(stages[0].stage);
            for (int i = 0; i < nstages-1; i++) {
                fStages[i].next = enum_to_Fn(stages[i+1].stage);
                fStages[i].ctx  = stages[i].ctx;
            }
            fStages[nstages-1].next = just_return;
            fStages[nstages-1].ctx  = stages[nstages-1].ctx;
        }

        void operator()(size_t x, size_t y, size_t n) {
            float dx[] = { 0,1,2,3,4,5,6,7 };
            SkNf X = SkNf(x) + SkNf::Load(dx) + 0.5f,
                 Y = SkNf(y) + 0.5f,
                _0 = SkNf(0),
                _1 = SkNf(1);

            while (n >= N) {
                fStart(fStages.data(), x*N, X,Y,_1,_0, _0,_0,_0,_0);
                X += (float)N;
                x += N;
                n -= N;
            }
            if (n) {
                fStart(fStages.data(), x*N+n, X,Y,_1,_0, _0,_0,_0,_0);
            }
        }

        Fn fStart = just_return;
        std::vector<Stage> fStages;
    };
}

namespace SK_OPTS_NS {

    SI std::function<void(size_t, size_t, size_t)>
    compile_pipeline(const SkRasterPipeline::Stage* stages, int nstages) {
        return Compiled{stages,nstages};
    }

    SI void run_pipeline(size_t x, size_t y, size_t n,
                         const SkRasterPipeline::Stage* stages, int nstages) {
        Compiled{stages,nstages}(x,y,n);
    }

}  // namespace SK_OPTS_NS

#undef SI
#undef STAGE
#undef RGBA_XFERMODE
#undef RGB_XFERMODE

#endif//SkRasterPipeline_opts_DEFINED
