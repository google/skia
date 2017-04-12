/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"     // SI, unaligned_load(), bit_cast(), C(), operator"" _i and _f.
#include "SkJumper_vectors.h"  // F, I32, U32, U16, U8, cast(), expand()

// Our fundamental vector depth is our pixel stride.
static const size_t kStride = sizeof(F) / sizeof(float);

// A reminder:
// Code guarded by defined(JUMPER) can assume that it will be compiled by Clang
// and that F, I32, etc. are kStride-deep ext_vector_types of the appropriate type.
// Otherwise, F, I32, etc. just alias the basic scalar types (and so kStride == 1).

// Another reminder:
// You can't generally use constants in this file except via C() or operator"" _i/_f.
// Not all constants can be generated using C() or _i/_f.  Stages read the rest from this struct.
using K = const SkJumper_constants;


// Let's start first with the mechanisms we use to build Stages.

// Our program is an array of void*, either
//   - 1 void* per stage with no context pointer, the next stage;
//   - 2 void* per stage with a context pointer, first the context pointer, then the next stage.

// load_and_inc() steps the program forward by 1 void*, returning that pointer.
SI void* load_and_inc(void**& program) {
#if defined(__GNUC__) && defined(__x86_64__)
    // If program is in %rsi (we try to make this likely) then this is a single instruction.
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));  // Write-only %rax, read-write %rsi.
    return rax;
#else
    // On ARM *program++ compiles into pretty ideal code without any handholding.
    return *program++;
#endif
}

// LazyCtx doesn't do anything unless you call operator T*(), encapsulating the logic
// from above that stages without a context pointer are represented by just 1 void*.
struct LazyCtx {
    void*   ptr;
    void**& program;

    explicit LazyCtx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }
};

// A little wrapper macro to name Stages differently depending on the instruction set.
// That lets us link together several options.
#if !defined(JUMPER)
    #define WRAP(name) sk_##name
#elif defined(__aarch64__)
    #define WRAP(name) sk_##name##_aarch64
#elif defined(__arm__)
    #define WRAP(name) sk_##name##_vfp4
#elif defined(__AVX2__)
    #define WRAP(name) sk_##name##_hsw
#elif defined(__AVX__)
    #define WRAP(name) sk_##name##_avx
#elif defined(__SSE4_1__)
    #define WRAP(name) sk_##name##_sse41
#elif defined(__SSE2__)
    #define WRAP(name) sk_##name##_sse2
#endif

// We're finally going to get to what a Stage function looks like!
// It's best to jump down to the #else case first, then to come back up here for AVX.

#if defined(JUMPER) && defined(__AVX__)
    // There's a big cost to switch between SSE and AVX, so we do a little
    // extra work to handle even the jagged <kStride tail in AVX mode.
    // Compared to normal stages, we maintain an extra tail register:
    //    tail == 0 ~~> work on a full kStride pixels
    //    tail != 0 ~~> work on only the first tail pixels
    // tail is always < kStride.
    using Stage = void(size_t x, void** program, K* k, size_t tail, F,F,F,F, F,F,F,F);

    #if defined(JUMPER) && defined(WIN)
    __attribute__((ms_abi))
    #endif
    extern "C" size_t WRAP(start_pipeline)(size_t x, void** program, K* k, size_t limit) {
        F v{};
        auto start = (Stage*)load_and_inc(program);
        while (x + kStride <= limit) {
            start(x,program,k,0,    v,v,v,v, v,v,v,v);
            x += kStride;
        }
        if (size_t tail = limit - x) {
            start(x,program,k,tail, v,v,v,v, v,v,v,v);
        }
        return limit;
    }

    #define STAGE(name)                                                           \
        SI void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,                \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);     \
        extern "C" void WRAP(name)(size_t x, void** program, K* k, size_t tail,   \
                                   F r, F g, F b, F a, F dr, F dg, F db, F da) {  \
            LazyCtx ctx(program);                                                 \
            name##_k(x,ctx,k,tail, r,g,b,a, dr,dg,db,da);                         \
            auto next = (Stage*)load_and_inc(program);                            \
            next(x,program,k,tail, r,g,b,a, dr,dg,db,da);                         \
        }                                                                         \
        SI void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,                \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)

#else
    // Other instruction sets (SSE, NEON, portable) can fall back on narrower
    // pipelines cheaply, which frees us to always assume tail==0.

    // Stages tail call between each other by following program as described above.
    // x is our induction variable, stepping forward kStride at a time.
    using Stage = void(size_t x, void** program, K* k, F,F,F,F, F,F,F,F);

    // On Windows, start_pipeline() has a normal Windows ABI, and then the rest is System V.
    #if defined(JUMPER) && defined(WIN)
    __attribute__((ms_abi))
    #endif
    extern "C" size_t WRAP(start_pipeline)(size_t x, void** program, K* k, size_t limit) {
        F v{};
        auto start = (Stage*)load_and_inc(program);
        while (x + kStride <= limit) {
            start(x,program,k, v,v,v,v, v,v,v,v);
            x += kStride;
        }
        return x;
    }

    // This STAGE macro makes it easier to write stages, handling all the Stage chaining for you.
    #define STAGE(name)                                                           \
        SI void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,                \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);     \
        extern "C" void WRAP(name)(size_t x, void** program, K* k,                \
                                   F r, F g, F b, F a, F dr, F dg, F db, F da) {  \
            LazyCtx ctx(program);                                                 \
            name##_k(x,ctx,k,0, r,g,b,a, dr,dg,db,da);                            \
            auto next = (Stage*)load_and_inc(program);                            \
            next(x,program,k, r,g,b,a, dr,dg,db,da);                              \
        }                                                                         \
        SI void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,                \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif

// just_return() is a simple no-op stage that only exists to end the chain,
// returning back up to start_pipeline(), and from there to the caller.
extern "C" void WRAP(just_return)(size_t, void**, K*, F,F,F,F, F,F,F,F) {}


// We could start defining normal Stages now.  But first, some helper functions.

// These load() and store() methods are tail-aware,
// but focus mainly on keeping the at-stride tail==0 case fast.

template <typename V, typename T>
SI V load(const T* src, size_t tail) {
#if defined(JUMPER)
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        V v{};  // Any inactive lanes are zeroed.
        switch (tail-1) {
            case 6: v[6] = src[6];
            case 5: v[5] = src[5];
            case 4: v[4] = src[4];
            case 3: v[3] = src[3];
            case 2: v[2] = src[2];
            case 1: v[1] = src[1];
            case 0: v[0] = src[0];
        }
        return v;
    }
#endif
    return unaligned_load<V>(src);
}

template <typename V, typename T>
SI void store(T* dst, V v, size_t tail) {
#if defined(JUMPER)
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        switch (tail-1) {
            case 6: dst[6] = v[6];
            case 5: dst[5] = v[5];
            case 4: dst[4] = v[4];
            case 3: dst[3] = v[3];
            case 2: dst[2] = v[2];
            case 1: dst[1] = v[1];
            case 0: dst[0] = v[0];
        }
        return;
    }
#endif
    memcpy(dst, &v, sizeof(v));
}

// This doesn't look strictly necessary, but without it Clang would generate load() using
// compiler-generated constants that we can't support.  This version doesn't need constants.
#if defined(JUMPER) && defined(__AVX__)
    template <>
    inline U8 load(const uint8_t* src, size_t tail) {
        if (__builtin_expect(tail, 0)) {
            uint64_t v = 0;
            size_t shift = 0;
            #pragma nounroll
            while (tail --> 0) {
                v |= (uint64_t)*src++ << shift;
                shift += 8;
            }
            return unaligned_load<U8>(&v);
        }
        return unaligned_load<U8>(src);
    }
#endif

// AVX2 adds some mask loads and stores that make for shorter, faster code.
#if defined(JUMPER) && defined(__AVX2__)
    SI U32 mask(size_t tail) {
        // We go a little out of our way to avoid needing large constant values here.

        // It's easiest to build the mask as 8 8-bit values, either 0x00 or 0xff.
        // Start fully on, then shift away lanes from the top until we've got our mask.
        uint64_t mask = 0xffffffffffffffff >> 8*(kStride-tail);

        // Sign-extend each mask lane to its full width, 0x00000000 or 0xffffffff.
        return _mm256_cvtepi8_epi32(_mm_cvtsi64_si128((int64_t)mask));
    }

    template <>
    inline U32 load(const uint32_t* src, size_t tail) {
        __builtin_assume(tail < kStride);
        if (__builtin_expect(tail, 0)) {
            return _mm256_maskload_epi32((const int*)src, mask(tail));
        }
        return unaligned_load<U32>(src);
    }

    template <>
    inline void store(uint32_t* dst, U32 v, size_t tail) {
        __builtin_assume(tail < kStride);
        if (__builtin_expect(tail, 0)) {
            return _mm256_maskstore_epi32((int*)dst, mask(tail), v);
        }
        memcpy(dst, &v, sizeof(v));
    }
#endif

SI F from_byte(U8 b) {
    return cast(expand(b)) * C(1/255.0f);
}
SI void from_565(U16 _565, F* r, F* g, F* b) {
    U32 wide = expand(_565);
    *r = cast(wide & C(31<<11)) * C(1.0f / (31<<11));
    *g = cast(wide & C(63<< 5)) * C(1.0f / (63<< 5));
    *b = cast(wide & C(31<< 0)) * C(1.0f / (31<< 0));
}
SI void from_4444(U16 _4444, F* r, F* g, F* b, F* a) {
    U32 wide = expand(_4444);
    *r = cast(wide & C(15<<12)) * C(1.0f / (15<<12));
    *g = cast(wide & C(15<< 8)) * C(1.0f / (15<< 8));
    *b = cast(wide & C(15<< 4)) * C(1.0f / (15<< 4));
    *a = cast(wide & C(15<< 0)) * C(1.0f / (15<< 0));
}
SI void from_8888(U32 _8888, F* r, F* g, F* b, F* a) {
    *r = cast((_8888      ) & 0xff_i) * C(1/255.0f);
    *g = cast((_8888 >>  8) & 0xff_i) * C(1/255.0f);
    *b = cast((_8888 >> 16) & 0xff_i) * C(1/255.0f);
    *a = cast((_8888 >> 24)         ) * C(1/255.0f);
}

template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkJumper_GatherCtx* ctx, F x, F y) {
    *ptr = (const T*)ctx->pixels;
    return trunc_(y)*ctx->stride + trunc_(x);
}

// Now finally, normal Stages!

STAGE(seed_shader) {
    auto y = *(const int*)ctx;

    // It's important for speed to explicitly cast(x) and cast(y),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.
    r = cast(x) + 0.5_f + unaligned_load<F>(k->iota);
    g = cast(y) + 0.5_f;
    b = 1.0_f;
    a = 0;
    dr = dg = db = da = 0;
}

STAGE(constant_color) {
    auto rgba = (const float*)ctx;
    r = rgba[0];
    g = rgba[1];
    b = rgba[2];
    a = rgba[3];
}

// Most blend modes apply the same logic to each channel.
#define BLEND_MODE(name)                       \
    SI F name##_channel(F s, F d, F sa, F da); \
    STAGE(name) {                              \
        r = name##_channel(r,dr,a,da);         \
        g = name##_channel(g,dg,a,da);         \
        b = name##_channel(b,db,a,da);         \
        a = name##_channel(a,da,a,da);         \
    }                                          \
    SI F name##_channel(F s, F d, F sa, F da)

SI F inv(F x) { return 1.0_f - x; }
SI F two(F x) { return x + x; }

BLEND_MODE(clear)    { return 0; }
BLEND_MODE(srcatop)  { return s*da + d*inv(sa); }
BLEND_MODE(dstatop)  { return d*sa + s*inv(da); }
BLEND_MODE(srcin)    { return s * da; }
BLEND_MODE(dstin)    { return d * sa; }
BLEND_MODE(srcout)   { return s * inv(da); }
BLEND_MODE(dstout)   { return d * inv(sa); }
BLEND_MODE(srcover)  { return mad(d, inv(sa), s); }
BLEND_MODE(dstover)  { return mad(s, inv(da), d); }

BLEND_MODE(modulate) { return s*d; }
BLEND_MODE(multiply) { return s*inv(da) + d*inv(sa) + s*d; }
BLEND_MODE(plus_)    { return s + d; }
BLEND_MODE(screen)   { return s + d - s*d; }
BLEND_MODE(xor_)     { return s*inv(da) + d*inv(sa); }
#undef BLEND_MODE

// Most other blend modes apply the same logic to colors, and srcover to alpha.
#define BLEND_MODE(name)                       \
    SI F name##_channel(F s, F d, F sa, F da); \
    STAGE(name) {                              \
        r = name##_channel(r,dr,a,da);         \
        g = name##_channel(g,dg,a,da);         \
        b = name##_channel(b,db,a,da);         \
        a = mad(da, inv(a), a);                \
    }                                          \
    SI F name##_channel(F s, F d, F sa, F da)

BLEND_MODE(darken)     { return s + d -     max(s*da, d*sa) ; }
BLEND_MODE(lighten)    { return s + d -     min(s*da, d*sa) ; }
BLEND_MODE(difference) { return s + d - two(min(s*da, d*sa)); }
BLEND_MODE(exclusion)  { return s + d - two(s*d); }

BLEND_MODE(colorburn) {
    return if_then_else(d == da, d + s*inv(da),
           if_then_else(s ==  0, s + d*inv(sa),
                                 sa*(da - min(da, (da-d)*sa/s)) + s*inv(da) + d*inv(sa)));
}
BLEND_MODE(colordodge) {
    return if_then_else(d ==  0, d + s*inv(da),
           if_then_else(s == sa, s + d*inv(sa),
                                 sa*min(da, (d*sa)/(sa - s)) + s*inv(da) + d*inv(sa)));
}
BLEND_MODE(hardlight) {
    return s*inv(da) + d*inv(sa)
         + if_then_else(two(s) <= sa, two(s*d), sa*da - two((da-d)*(sa-s)));
}
BLEND_MODE(overlay) {
    return s*inv(da) + d*inv(sa)
         + if_then_else(two(d) <= da, two(s*d), sa*da - two((da-d)*(sa-s)));
}

BLEND_MODE(softlight) {
    F m  = if_then_else(da > 0, d / da, 0),
      s2 = two(s),
      m4 = two(two(m));

    // The logic forks three ways:
    //    1. dark src?
    //    2. light src, dark dst?
    //    3. light src, light dst?
    F darkSrc = d*(sa + (s2 - sa)*(1.0_f - m)),      // Used in case 1.
      darkDst = (m4*m4 + m4)*(m - 1.0_f) + 7.0_f*m,  // Used in case 2.
      liteDst = rcp(rsqrt(m)) - m,                   // Used in case 3.
      liteSrc = d*sa + da*(s2 - sa) * if_then_else(two(two(d)) <= da, darkDst, liteDst); // 2 or 3?
    return s*inv(da) + d*inv(sa) + if_then_else(s2 <= sa, darkSrc, liteSrc);      // 1 or (2 or 3)?
}
#undef BLEND_MODE

STAGE(clamp_0) {
    r = max(r, 0);
    g = max(g, 0);
    b = max(b, 0);
    a = max(a, 0);
}

STAGE(clamp_1) {
    r = min(r, 1.0_f);
    g = min(g, 1.0_f);
    b = min(b, 1.0_f);
    a = min(a, 1.0_f);
}

STAGE(clamp_a) {
    a = min(a, 1.0_f);
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}

STAGE(set_rgb) {
    auto rgb = (const float*)ctx;
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}
STAGE(swap_rb) {
    auto tmp = r;
    r = b;
    b = tmp;
}

STAGE(swap) {
    auto swap = [](F& v, F& dv) {
        auto tmp = v;
        v = dv;
        dv = tmp;
    };
    swap(r, dr);
    swap(g, dg);
    swap(b, db);
    swap(a, da);
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

STAGE(premul) {
    r = r * a;
    g = g * a;
    b = b * a;
}
STAGE(unpremul) {
    auto scale = if_then_else(a == 0, 0, 1.0_f / a);
    r = r * scale;
    g = g * scale;
    b = b * scale;
}

STAGE(from_srgb) {
    auto fn = [&](F s) {
        auto lo = s * C(1/12.92f);
        auto hi = mad(s*s, mad(s, 0.3000_f, 0.6975_f), 0.0025_f);
        return if_then_else(s < 0.055_f, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}
STAGE(to_srgb) {
    auto fn = [&](F l) {
        F sqrt = rcp  (rsqrt(l)),
          ftrt = rsqrt(rsqrt(l));
        auto lo = l * 12.46_f;
        auto hi = min(1.0_f, mad(0.411192_f, ftrt,
                             mad(0.689206_f, sqrt, -0.0988_f)));
        return if_then_else(l < 0.0043_f, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(from_2dot2) {
    auto fn = [](F x) {
        // x^(141/64) = x^(2.20312) is a great approximation of the true value, x^(2.2).
        // (note: x^(35/16) = x^(2.1875) is an okay one as well and would be quicker)
        F x16 = rsqrt(rsqrt(rsqrt(rsqrt(x)))),    // x^(1/16) = x^(4/64);
          x64 = rsqrt(rsqrt(x16));                // x^(1/64)

        // 141/64 = 128/64 + 12/64 + 1/64
        return max((x*x) * (x16*x16*x16) * x64, 0);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}
STAGE(to_2dot2) {
    auto fn = [](F x) {
        // x^(29/64) is a very good approximation of the true value, x^(1/2.2).
        F x2  = rsqrt(x),                         // x^(-1/2)
          x32 = rsqrt(rsqrt(rsqrt(rsqrt(x2)))),   // x^(-1/32)
          x64 = rsqrt(x32);                       // x^(+1/64)

        // 29/64 = 32/64 - 2/64 - 1/64
        return max(rcp(x2) * x32 * rcp(x64), 0);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(rgb_to_hsl) {
    F mx = max(max(r,g), b),
      mn = min(min(r,g), b),
      d = mx - mn,
      d_rcp = 1.0_f / d;

    F h = C(1/6.0f) *
          if_then_else(mx == mn, 0,
          if_then_else(mx ==  r, (g-b)*d_rcp + if_then_else(g < b, 6.0_f, 0),
          if_then_else(mx ==  g, (b-r)*d_rcp + 2.0_f,
                                 (r-g)*d_rcp + 4.0_f)));

    F l = (mx + mn) * 0.5_f;
    F s = if_then_else(mx == mn, 0,
                       d / if_then_else(l > 0.5_f, 2.0_f-mx-mn, mx+mn));

    r = h;
    g = s;
    b = l;
}
STAGE(hsl_to_rgb) {
    F h = r,
      s = g,
      l = b;

    F q = if_then_else(l < 0.5_f, l*(1.0_f + s), l + s - l*s),
      p = 2.0_f*l - q;

    auto hue_to_rgb = [&](F t) {
        F t2 = if_then_else(t < 0.0_f, t + 1.0_f,
               if_then_else(t > 1.0_f, t - 1.0_f,
                                       t));

        return if_then_else(t2 < C(1/6.0f),  p + (q-p)*6.0_f*t,
               if_then_else(t2 < C(3/6.0f),  q,
               if_then_else(t2 < C(4/6.0f),  p + (q-p)*6.0_f*(C(4/6.0f) - t2),
                                             p)));
    };

    r = if_then_else(s == 0, l, hue_to_rgb(h + C(1/3.0f)));
    g = if_then_else(s == 0, l, hue_to_rgb(h            ));
    b = if_then_else(s == 0, l, hue_to_rgb(h - C(1/3.0f)));
}

STAGE(scale_1_float) {
    auto c = *(const float*)ctx;

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}

SI F lerp(F from, F to, F t) {
    return mad(to-from, t, from);
}

STAGE(lerp_1_float) {
    auto c = *(const float*)ctx;

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_565) {
    auto ptr = *(const uint16_t**)ctx + x;

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0_f;
}

STAGE(load_tables) {
    struct Ctx {
        const uint32_t* src;
        const float *r, *g, *b;
    };
    auto c = (const Ctx*)ctx;

    auto px = load<U32>(c->src + x, tail);
    r = gather(c->r, (px      ) & 0xff_i);
    g = gather(c->g, (px >>  8) & 0xff_i);
    b = gather(c->b, (px >> 16) & 0xff_i);
    a = cast(        (px >> 24)) * C(1/255.0f);
}

STAGE(byte_tables) {
    struct Tables { const uint8_t *r, *g, *b, *a; };
    auto tables = (const Tables*)ctx;

    r = from_byte(gather(tables->r, round(r, 255.0_f)));
    g = from_byte(gather(tables->g, round(g, 255.0_f)));
    b = from_byte(gather(tables->b, round(b, 255.0_f)));
    a = from_byte(gather(tables->a, round(a, 255.0_f)));
}

STAGE(byte_tables_rgb) {
    struct Tables { const uint8_t *r, *g, *b; int n; };
    auto tables = (const Tables*)ctx;

    F scale = tables->n - 1;
    r = from_byte(gather(tables->r, round(r, scale)));
    g = from_byte(gather(tables->g, round(g, scale)));
    b = from_byte(gather(tables->b, round(b, scale)));
}

STAGE(load_a8) {
    auto ptr = *(const uint8_t**)ctx + x;

    r = g = b = 0.0f;
    a = from_byte(load<U8>(ptr, tail));
}
STAGE(gather_a8) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = 0.0f;
    a = from_byte(gather(ptr, ix));
}
STAGE(store_a8) {
    auto ptr = *(uint8_t**)ctx + x;

    U8 packed = pack(pack(round(a, 255.0_f)));
    store(ptr, packed, tail);
}

STAGE(load_g8) {
    auto ptr = *(const uint8_t**)ctx + x;

    r = g = b = from_byte(load<U8>(ptr, tail));
    a = 1.0_f;
}
STAGE(gather_g8) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = from_byte(gather(ptr, ix));
    a = 1.0_f;
}

STAGE(gather_i8) {
    auto c = (const SkJumper_GatherCtx*)ctx;
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    ix = expand(gather(ptr, ix));
    from_8888(gather(c->ctable, ix), &r,&g,&b,&a);
}

STAGE(load_565) {
    auto ptr = *(const uint16_t**)ctx + x;

    from_565(load<U16>(ptr, tail), &r,&g,&b);
    a = 1.0_f;
}
STAGE(gather_565) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_565(gather(ptr, ix), &r,&g,&b);
    a = 1.0_f;
}
STAGE(store_565) {
    auto ptr = *(uint16_t**)ctx + x;

    U16 px = pack( round(r, 31.0_f) << 11
                 | round(g, 63.0_f) <<  5
                 | round(b, 31.0_f)      );
    store(ptr, px, tail);
}

STAGE(load_4444) {
    auto ptr = *(const uint16_t**)ctx + x;
    from_4444(load<U16>(ptr, tail), &r,&g,&b,&a);
}
STAGE(gather_4444) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_4444(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_4444) {
    auto ptr = *(uint16_t**)ctx + x;
    U16 px = pack( round(r, 15.0_f) << 12
                 | round(g, 15.0_f) <<  8
                 | round(b, 15.0_f) <<  4
                 | round(a, 15.0_f)      );
    store(ptr, px, tail);
}

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(gather_8888) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_8888(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;

    U32 px = round(r, 255.0_f)
           | round(g, 255.0_f) <<  8
           | round(b, 255.0_f) << 16
           | round(a, 255.0_f) << 24;
    store(ptr, px, tail);
}

STAGE(load_f16) {
    auto ptr = *(const uint64_t**)ctx + x;

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(gather_f16) {
    const uint64_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    auto px = gather(ptr, ix);

    U16 R,G,B,A;
    load4((const uint16_t*)&px,0, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(store_f16) {
    auto ptr = *(uint64_t**)ctx + x;
    store4((uint16_t*)ptr,tail, to_half(r)
                              , to_half(g)
                              , to_half(b)
                              , to_half(a));
}

STAGE(load_u16_be) {
    auto ptr = *(const uint64_t**)ctx + x;

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);

    r = C(1/65535.0f) * cast(expand(bswap(R)));
    g = C(1/65535.0f) * cast(expand(bswap(G)));
    b = C(1/65535.0f) * cast(expand(bswap(B)));
    a = C(1/65535.0f) * cast(expand(bswap(A)));
}
STAGE(store_u16_be) {
    auto ptr = *(uint64_t**)ctx + x;

    U16 R = bswap(pack(round(r, 65535.0_f))),
        G = bswap(pack(round(g, 65535.0_f))),
        B = bswap(pack(round(b, 65535.0_f))),
        A = bswap(pack(round(a, 65535.0_f)));

    store4((uint16_t*)ptr,tail, R,G,B,A);
}

STAGE(load_f32) {
    auto ptr = *(const float**)ctx + 4*x;
    load4(ptr,tail, &r,&g,&b,&a);
}
STAGE(store_f32) {
    auto ptr = *(float**)ctx + 4*x;
    store4(ptr,tail, r,g,b,a);
}

SI F ulp_before(F v) {
    return bit_cast<F>(bit_cast<U32>(v) + U32(0xffffffff));
}
SI F clamp(F v, float limit) {
    v = max(0, v);
    return min(v, ulp_before(limit));
}
SI F repeat(F v, float limit) {
    v = v - floor_(v/limit)*limit;
    return min(v, ulp_before(limit));
}
SI F mirror(F v, float limit) {
    v = abs_( (v-limit) - (limit+limit)*floor_((v-limit)/(limit+limit)) - limit );
    return min(v, ulp_before(limit));
}
STAGE(clamp_x)  { r = clamp (r, *(const float*)ctx); }
STAGE(clamp_y)  { g = clamp (g, *(const float*)ctx); }
STAGE(repeat_x) { r = repeat(r, *(const float*)ctx); }
STAGE(repeat_y) { g = repeat(g, *(const float*)ctx); }
STAGE(mirror_x) { r = mirror(r, *(const float*)ctx); }
STAGE(mirror_y) { g = mirror(g, *(const float*)ctx); }

STAGE(luminance_to_alpha) {
    a = r*0.2126_f + g*0.7152_f + b*0.0722_f;
    r = g = b = 0;
}

STAGE(matrix_2x3) {
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[2], m[4])),
         G = mad(r,m[1], mad(g,m[3], m[5]));
    r = R;
    g = G;
}
STAGE(matrix_3x4) {
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[3], mad(b,m[6], m[ 9]))),
         G = mad(r,m[1], mad(g,m[4], mad(b,m[7], m[10]))),
         B = mad(r,m[2], mad(g,m[5], mad(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}
STAGE(matrix_4x5) {
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[4], mad(b,m[ 8], mad(a,m[12], m[16])))),
         G = mad(r,m[1], mad(g,m[5], mad(b,m[ 9], mad(a,m[13], m[17])))),
         B = mad(r,m[2], mad(g,m[6], mad(b,m[10], mad(a,m[14], m[18])))),
         A = mad(r,m[3], mad(g,m[7], mad(b,m[11], mad(a,m[15], m[19]))));
    r = R;
    g = G;
    b = B;
    a = A;
}
STAGE(matrix_perspective) {
    // N.B. Unlike the other matrix_ stages, this matrix is row-major.
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[1], m[2])),
         G = mad(r,m[3], mad(g,m[4], m[5])),
         Z = mad(r,m[6], mad(g,m[7], m[8]));
    r = R * rcp(Z);
    g = G * rcp(Z);
}

STAGE(linear_gradient) {
    struct Stop { float pos; float f[4], b[4]; };
    struct Ctx { size_t n; Stop *stops; float start[4]; };

    auto c = (const Ctx*)ctx;
    F fr = 0, fg = 0, fb = 0, fa = 0;
    F br = c->start[0],
      bg = c->start[1],
      bb = c->start[2],
      ba = c->start[3];
    auto t = r;
    for (size_t i = 0; i < c->n; i++) {
        fr = if_then_else(t < c->stops[i].pos, fr, c->stops[i].f[0]);
        fg = if_then_else(t < c->stops[i].pos, fg, c->stops[i].f[1]);
        fb = if_then_else(t < c->stops[i].pos, fb, c->stops[i].f[2]);
        fa = if_then_else(t < c->stops[i].pos, fa, c->stops[i].f[3]);
        br = if_then_else(t < c->stops[i].pos, br, c->stops[i].b[0]);
        bg = if_then_else(t < c->stops[i].pos, bg, c->stops[i].b[1]);
        bb = if_then_else(t < c->stops[i].pos, bb, c->stops[i].b[2]);
        ba = if_then_else(t < c->stops[i].pos, ba, c->stops[i].b[3]);
    }

    r = mad(t, fr, br);
    g = mad(t, fg, bg);
    b = mad(t, fb, bb);
    a = mad(t, fa, ba);
}

STAGE(linear_gradient_2stops) {
    struct Ctx { float f[4], b[4]; };
    auto c = (const Ctx*)ctx;

    auto t = r;
    r = mad(t, c->f[0], c->b[0]);
    g = mad(t, c->f[1], c->b[1]);
    b = mad(t, c->f[2], c->b[2]);
    a = mad(t, c->f[3], c->b[3]);
}

STAGE(save_xy) {
    auto c = (SkJumper_SamplerCtx*)ctx;

    // Whether bilinear or bicubic, all sample points are at the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y) at (0.5,0.5) off-center.
    auto fract = [](F v) { return v - floor_(v); };
    F fx = fract(r + 0.5_f),
      fy = fract(g + 0.5_f);

    // Samplers will need to load x and fx, or y and fy.
    memcpy(c->x,  &r,  sizeof(F));
    memcpy(c->y,  &g,  sizeof(F));
    memcpy(c->fx, &fx, sizeof(F));
    memcpy(c->fy, &fy, sizeof(F));
}

STAGE(accumulate) {
    auto c = (const SkJumper_SamplerCtx*)ctx;

    // Bilinear and bicubic filters are both separable, so we produce independent contributions
    // from x and y, multiplying them together here to get each pixel's total scale factor.
    auto scale = unaligned_load<F>(c->scalex)
               * unaligned_load<F>(c->scaley);
    dr = mad(scale, r, dr);
    dg = mad(scale, g, dg);
    db = mad(scale, b, db);
    da = mad(scale, a, da);
}

// In bilinear interpolation, the 4 pixels at +/- 0.5 offsets from the sample pixel center
// are combined in direct proportion to their area overlapping that logical query pixel.
// At positive offsets, the x-axis contribution to that rectangle is fx, or (1-fx) at negative x.
// The y-axis is symmetric.

template <int kScale>
SI void bilinear_x(SkJumper_SamplerCtx* ctx, F* x) {
    *x = unaligned_load<F>(ctx->x) + C(kScale * 0.5f);
    F fx = unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -1) { scalex = 1.0_f - fx; }
    if (kScale == +1) { scalex =         fx; }
    memcpy(ctx->scalex, &scalex, sizeof(F));
}
template <int kScale>
SI void bilinear_y(SkJumper_SamplerCtx* ctx, F* y) {
    *y = unaligned_load<F>(ctx->y) + C(kScale * 0.5f);
    F fy = unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -1) { scaley = 1.0_f - fy; }
    if (kScale == +1) { scaley =         fy; }
    memcpy(ctx->scaley, &scaley, sizeof(F));
}

STAGE(bilinear_nx) { bilinear_x<-1>(ctx, &r); }
STAGE(bilinear_px) { bilinear_x<+1>(ctx, &r); }
STAGE(bilinear_ny) { bilinear_y<-1>(ctx, &g); }
STAGE(bilinear_py) { bilinear_y<+1>(ctx, &g); }


// In bicubic interpolation, the 16 pixels and +/- 0.5 and +/- 1.5 offsets from the sample
// pixel center are combined with a non-uniform cubic filter, with higher values near the center.
//
// We break this function into two parts, one for near 0.5 offsets and one for far 1.5 offsets.
// See GrCubicEffect for details of this particular filter.

SI F bicubic_near(F t) {
    // 1/18 + 9/18t + 27/18t^2 - 21/18t^3 == t ( t ( -21/18t + 27/18) + 9/18) + 1/18
    return mad(t, mad(t, mad(C(-21/18.0f), t, C(27/18.0f)), C(9/18.0f)), C(1/18.0f));
}
SI F bicubic_far(F t) {
    // 0/18 + 0/18*t - 6/18t^2 + 7/18t^3 == t^2 (7/18t - 6/18)
    return (t*t)*mad(C(7/18.0f), t, C(-6/18.0f));
}

template <int kScale>
SI void bicubic_x(SkJumper_SamplerCtx* ctx, F* x) {
    *x = unaligned_load<F>(ctx->x) + C(kScale * 0.5f);
    F fx = unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -3) { scalex = bicubic_far (1.0_f - fx); }
    if (kScale == -1) { scalex = bicubic_near(1.0_f - fx); }
    if (kScale == +1) { scalex = bicubic_near(        fx); }
    if (kScale == +3) { scalex = bicubic_far (        fx); }
    memcpy(ctx->scalex, &scalex, sizeof(F));
}
template <int kScale>
SI void bicubic_y(SkJumper_SamplerCtx* ctx, F* y) {
    *y = unaligned_load<F>(ctx->y) + C(kScale * 0.5f);
    F fy = unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -3) { scaley = bicubic_far (1.0_f - fy); }
    if (kScale == -1) { scaley = bicubic_near(1.0_f - fy); }
    if (kScale == +1) { scaley = bicubic_near(        fy); }
    if (kScale == +3) { scaley = bicubic_far (        fy); }
    memcpy(ctx->scaley, &scaley, sizeof(F));
}

STAGE(bicubic_n3x) { bicubic_x<-3>(ctx, &r); }
STAGE(bicubic_n1x) { bicubic_x<-1>(ctx, &r); }
STAGE(bicubic_p1x) { bicubic_x<+1>(ctx, &r); }
STAGE(bicubic_p3x) { bicubic_x<+3>(ctx, &r); }

STAGE(bicubic_n3y) { bicubic_y<-3>(ctx, &g); }
STAGE(bicubic_n1y) { bicubic_y<-1>(ctx, &g); }
STAGE(bicubic_p1y) { bicubic_y<+1>(ctx, &g); }
STAGE(bicubic_p3y) { bicubic_y<+3>(ctx, &g); }
