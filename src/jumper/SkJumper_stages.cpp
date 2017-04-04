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

// LazyCtx doesn't do anything unless you call operator T*() or load(), encapsulating the
// logic from above that stages without a context pointer are represented by just 1 void*.
struct LazyCtx {
    void*   ptr;
    void**& program;

    explicit LazyCtx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }

    template <typename T>
    T load() {
        if (!ptr) { ptr = load_and_inc(program); }
        return unaligned_load<T>(ptr);
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


// We could start defining normal Stages now.  But first, some helper functions and types.

// Sometimes we want to work with 4 floats directly, regardless of the depth of the F vector.
#if defined(JUMPER)
    using F4 = float __attribute__((ext_vector_type(4)));
#else
    struct F4 {
        float vals[4];
        float operator[](int i) const { return vals[i]; }
    };
#endif

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
    auto rgba = ctx.load<F4>();
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
    auto c = cast(expand(scales)) * C(1/255.0f);

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
    auto c = cast(expand(scales)) * C(1/255.0f);

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

STAGE(load_a8) {
    auto ptr = *(const uint8_t**)ctx + x;

    r = g = b = 0.0f;
    a = cast(expand(load<U8>(ptr, tail))) * C(1/255.0f);
}
STAGE(store_a8) {
    auto ptr = *(uint8_t**)ctx + x;

    U8 packed = pack(pack(round(a, 255.0_f)));
    store(ptr, packed, tail);
}

STAGE(load_g8) {
    auto ptr = *(const uint8_t**)ctx + x;

    r = g = b = cast(expand(load<U8>(ptr, tail))) * C(1/255.0f);
    a = 1.0_f;
}

STAGE(load_565) {
    auto ptr = *(const uint16_t**)ctx + x;

    from_565(load<U16>(ptr, tail), &r,&g,&b);
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

    auto px = load<U32>(ptr, tail);
    r = cast((px      ) & 0xff_i) * C(1/255.0f);
    g = cast((px >>  8) & 0xff_i) * C(1/255.0f);
    b = cast((px >> 16) & 0xff_i) * C(1/255.0f);
    a = cast((px >> 24)         ) * C(1/255.0f);
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
    load4(ptr,tail, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}

STAGE(store_f16) {
    auto ptr = *(uint64_t**)ctx + x;

#if !defined(JUMPER)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    };
    auto rgba = (int16_t*)ptr;
    rgba[0] = float_to_half(r);
    rgba[1] = float_to_half(g);
    rgba[2] = float_to_half(b);
    rgba[3] = float_to_half(a);
#elif defined(__aarch64__)
    float16x4x4_t halfs = {{
        vcvt_f16_f32(r),
        vcvt_f16_f32(g),
        vcvt_f16_f32(b),
        vcvt_f16_f32(a),
    }};
    vst4_f16((float16_t*)ptr, halfs);
#elif defined(__arm__)
    float16x4x2_t rb_ga = {{
        vcvt_f16_f32(float32x4_t{r[0], b[0], r[1], b[1]}),
        vcvt_f16_f32(float32x4_t{g[0], a[0], g[1], a[1]}),
    }};
    vst2_f16((float16_t*)ptr, rb_ga);
#elif defined(__AVX2__) && defined(__FMA__) && defined(__F16C__)
    auto R = _mm256_cvtps_ph(r, _MM_FROUND_CUR_DIRECTION),
         G = _mm256_cvtps_ph(g, _MM_FROUND_CUR_DIRECTION),
         B = _mm256_cvtps_ph(b, _MM_FROUND_CUR_DIRECTION),
         A = _mm256_cvtps_ph(a, _MM_FROUND_CUR_DIRECTION);

    auto rg0123 = _mm_unpacklo_epi16(R, G),  // r0 g0 r1 g1 r2 g2 r3 g3
         rg4567 = _mm_unpackhi_epi16(R, G),  // r4 g4 r5 g5 r6 g6 r7 g7
         ba0123 = _mm_unpacklo_epi16(B, A),
         ba4567 = _mm_unpackhi_epi16(B, A);

    auto _01 = _mm_unpacklo_epi32(rg0123, ba0123),
         _23 = _mm_unpackhi_epi32(rg0123, ba0123),
         _45 = _mm_unpacklo_epi32(rg4567, ba4567),
         _67 = _mm_unpackhi_epi32(rg4567, ba4567);

    if (__builtin_expect(tail,0)) {
        auto dst = (double*)ptr;
        if (tail > 0) { _mm_storel_pd(dst+0, _01); }
        if (tail > 1) { _mm_storeh_pd(dst+1, _01); }
        if (tail > 2) { _mm_storel_pd(dst+2, _23); }
        if (tail > 3) { _mm_storeh_pd(dst+3, _23); }
        if (tail > 4) { _mm_storel_pd(dst+4, _45); }
        if (tail > 5) { _mm_storeh_pd(dst+5, _45); }
        if (tail > 6) { _mm_storel_pd(dst+6, _67); }
    } else {
        _mm_storeu_si128((__m128i*)ptr + 0, _01);
        _mm_storeu_si128((__m128i*)ptr + 1, _23);
        _mm_storeu_si128((__m128i*)ptr + 2, _45);
        _mm_storeu_si128((__m128i*)ptr + 3, _67);
    }
#elif defined(__AVX__)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    };
    U32 R = float_to_half(r),
        G = float_to_half(g),
        B = float_to_half(b),
        A = float_to_half(a);
    auto r0123 = _mm256_extractf128_si256(R, 0),
         r4567 = _mm256_extractf128_si256(R, 1),
         g0123 = _mm256_extractf128_si256(G, 0),
         g4567 = _mm256_extractf128_si256(G, 1),
         b0123 = _mm256_extractf128_si256(B, 0),
         b4567 = _mm256_extractf128_si256(B, 1),
         a0123 = _mm256_extractf128_si256(A, 0),
         a4567 = _mm256_extractf128_si256(A, 1);
    auto rg0123 = r0123 | _mm_slli_si128(g0123,2),
         rg4567 = r4567 | _mm_slli_si128(g4567,2),
         ba0123 = b0123 | _mm_slli_si128(a0123,2),
         ba4567 = b4567 | _mm_slli_si128(a4567,2);

    auto _01 = _mm_unpacklo_epi32(rg0123, ba0123),
         _23 = _mm_unpackhi_epi32(rg0123, ba0123),
         _45 = _mm_unpacklo_epi32(rg4567, ba4567),
         _67 = _mm_unpackhi_epi32(rg4567, ba4567);

    if (__builtin_expect(tail,0)) {
        auto dst = (double*)ptr;
        if (tail > 0) { _mm_storel_pd(dst+0, _01); }
        if (tail > 1) { _mm_storeh_pd(dst+1, _01); }
        if (tail > 2) { _mm_storel_pd(dst+2, _23); }
        if (tail > 3) { _mm_storeh_pd(dst+3, _23); }
        if (tail > 4) { _mm_storel_pd(dst+4, _45); }
        if (tail > 5) { _mm_storeh_pd(dst+5, _45); }
        if (tail > 6) { _mm_storel_pd(dst+6, _67); }
    } else {
        _mm_storeu_si128((__m128i*)ptr + 0, _01);
        _mm_storeu_si128((__m128i*)ptr + 1, _23);
        _mm_storeu_si128((__m128i*)ptr + 2, _45);
        _mm_storeu_si128((__m128i*)ptr + 3, _67);
    }
#elif defined(__SSE2__)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    };
    U32 R = float_to_half(r),
        G = float_to_half(g),
        B = float_to_half(b),
        A = float_to_half(a);
    U32 rg = R | _mm_slli_si128(G,2),
        ba = B | _mm_slli_si128(A,2);
    _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg, ba));
    _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg, ba));
#endif
}

STAGE(store_f32) {
    auto ptr = *(float**)ctx + 4*x;

#if !defined(JUMPER)
    ptr[0] = r;
    ptr[1] = g;
    ptr[2] = b;
    ptr[3] = a;
#elif defined(__aarch64__)
    vst4q_f32(ptr, (float32x4x4_t{{r,g,b,a}}));
#elif defined(__arm__)
    vst4_f32(ptr, (float32x2x4_t{{r,g,b,a}}));
#elif defined(__AVX__)
    F rg0145 = _mm256_unpacklo_ps(r, g),  // r0 g0 r1 g1 | r4 g4 r5 g5
      rg2367 = _mm256_unpackhi_ps(r, g),  // r2 ...      | r6 ...
      ba0145 = _mm256_unpacklo_ps(b, a),  // b0 a0 b1 a1 | b4 a4 b5 a5
      ba2367 = _mm256_unpackhi_ps(b, a);  // b2 ...      | b6 ...

    F _04 = _mm256_unpacklo_pd(rg0145, ba0145),  // r0 g0 b0 a0 | r4 g4 b4 a4
      _15 = _mm256_unpackhi_pd(rg0145, ba0145),  // r1 ...      | r5 ...
      _26 = _mm256_unpacklo_pd(rg2367, ba2367),  // r2 ...      | r6 ...
      _37 = _mm256_unpackhi_pd(rg2367, ba2367);  // r3 ...      | r7 ...

    if (__builtin_expect(tail, 0)) {
        if (tail > 0) { _mm_storeu_ps(ptr+ 0, _mm256_extractf128_ps(_04, 0)); }
        if (tail > 1) { _mm_storeu_ps(ptr+ 4, _mm256_extractf128_ps(_15, 0)); }
        if (tail > 2) { _mm_storeu_ps(ptr+ 8, _mm256_extractf128_ps(_26, 0)); }
        if (tail > 3) { _mm_storeu_ps(ptr+12, _mm256_extractf128_ps(_37, 0)); }
        if (tail > 4) { _mm_storeu_ps(ptr+16, _mm256_extractf128_ps(_04, 1)); }
        if (tail > 5) { _mm_storeu_ps(ptr+20, _mm256_extractf128_ps(_15, 1)); }
        if (tail > 6) { _mm_storeu_ps(ptr+24, _mm256_extractf128_ps(_26, 1)); }
    } else {
        F _01 = _mm256_permute2f128_ps(_04, _15, 32),  // 32 == 0010 0000 == lo, lo
          _23 = _mm256_permute2f128_ps(_26, _37, 32),
          _45 = _mm256_permute2f128_ps(_04, _15, 49),  // 49 == 0011 0001 == hi, hi
          _67 = _mm256_permute2f128_ps(_26, _37, 49);
        _mm256_storeu_ps(ptr+ 0, _01);
        _mm256_storeu_ps(ptr+ 8, _23);
        _mm256_storeu_ps(ptr+16, _45);
        _mm256_storeu_ps(ptr+24, _67);
    }
#elif defined(__SSE2__)
    auto v0 = r, v1 = g, v2 = b, v3 = a;
    _MM_TRANSPOSE4_PS(v0, v1, v2, v3);
    memcpy(ptr+ 0, &v0, sizeof(v0));
    memcpy(ptr+ 4, &v1, sizeof(v1));
    memcpy(ptr+ 8, &v2, sizeof(v2));
    memcpy(ptr+12, &v3, sizeof(v3));
#endif
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

STAGE(linear_gradient_2stops) {
    struct Ctx { F4 c0, dc; };
    auto c = ctx.load<Ctx>();

    auto t = r;
    r = mad(t, c.dc[0], c.c0[0]);
    g = mad(t, c.dc[1], c.c0[1]);
    b = mad(t, c.dc[2], c.c0[2]);
    a = mad(t, c.dc[3], c.c0[3]);
}
