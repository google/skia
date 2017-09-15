/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"     // SI, unaligned_load(), bit_cast()
#include "SkJumper_vectors.h"  // F, I32, U32, U16, U8, cast(), expand()

// Our fundamental vector depth is our pixel stride.
static const size_t kStride = sizeof(F) / sizeof(float);

// A reminder:
// When defined(JUMPER_IS_SCALAR), F, I32, etc. are normal scalar types and kStride is 1.
// When not, F, I32, etc. are kStride-depp Clang ext_vector_type vectors of the appropriate type.

// A little wrapper macro to name Stages differently depending on the instruction set.
// That lets us link together several options.
#if !defined(JUMPER_IS_OFFLINE)
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
//    tail == 0 ~~> work on a full kStride pixels
//    tail != 0 ~~> work on only the first tail pixels
// tail is always < kStride.

#if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
    // On 32-bit x86 we've only got 8 xmm registers, so we keep the 4 hottest (r,g,b,a)
    // in registers and the d-registers on the stack (giving us 4 temporary registers).
    // General-purpose registers are also tight, so we put most of those on the stack too.
    // On ARMv7, we do the same so that we can make the r,g,b,a vectors wider.
    struct Params {
        size_t x, y, tail;
        F dr,dg,db,da;
    };
    using Stage = void(Params*, void** program, F r, F g, F b, F a);

#else
    // We keep program the second argument, so that it's passed in rsi for load_and_inc().
    using Stage = void(size_t tail, void** program, size_t x, size_t y, F,F,F,F, F,F,F,F);
#endif

#if defined(JUMPER_IS_AVX) || defined(JUMPER_IS_AVX2)
    // We really want to make sure all paths go through this function's (implicit) vzeroupper.
    // If they don't, we'll experience severe slowdowns when we first use SSE instructions again.
    __attribute__((disable_tail_calls))
#endif
MAYBE_MSABI
extern "C" void WRAP(start_pipeline)(size_t x, size_t y, size_t xlimit, size_t ylimit,
                                     void** program) {
#if defined(JUMPER_IS_OFFLINE)
    F v;    // Really no need to intialize.
#else
    F v{};  // Compilers tend to whine about this, so it's easiest to just zero.
#endif
    auto start = (Stage*)load_and_inc(program);
    const size_t x0 = x;
    for (; y < ylimit; y++) {
    #if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
        Params params = { x0,y,0, v,v,v,v };
        while (params.x + kStride <= xlimit) {
            start(&params,program, v,v,v,v);
            params.x += kStride;
        }
        if (size_t tail = xlimit - params.x) {
            params.tail = tail;
            start(&params,program, v,v,v,v);
        }
    #else
        x = x0;
        while (x + kStride <= xlimit) {
            start(0,program,x,y,    v,v,v,v, v,v,v,v);
            x += kStride;
        }
        if (size_t tail = xlimit - x) {
            start(tail,program,x,y, v,v,v,v, v,v,v,v);
        }
    #endif
    }
}

#if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
    #define STAGE(name)                                                               \
        SI void name##_k(Ctx ctx, size_t x, size_t y, size_t tail,                    \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);         \
        extern "C" void WRAP(name)(Params* params, void** program,                    \
                                   F r, F g, F b, F a) {                              \
            Ctx ctx(program);                                                         \
            name##_k(ctx,params->x,params->y,params->tail, r,g,b,a,                   \
                     params->dr, params->dg, params->db, params->da);                 \
            auto next = (Stage*)load_and_inc(program);                                \
            next(params,program, r,g,b,a);                                            \
        }                                                                             \
        SI void name##_k(Ctx ctx, size_t x, size_t y, size_t tail,                    \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#else
    #define STAGE(name)                                                               \
        SI void name##_k(Ctx ctx, size_t x, size_t y, size_t tail,                    \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);         \
        extern "C" void WRAP(name)(size_t tail, void** program, size_t x, size_t y,   \
                                   F r, F g, F b, F a, F dr, F dg, F db, F da) {      \
            Ctx ctx(program);                                                         \
            name##_k(ctx,x,y,tail, r,g,b,a, dr,dg,db,da);                             \
            auto next = (Stage*)load_and_inc(program);                                \
            next(tail,program,x,y, r,g,b,a, dr,dg,db,da);                             \
        }                                                                             \
        SI void name##_k(Ctx ctx, size_t x, size_t y, size_t tail,                    \
                         F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif


// just_return() is a simple no-op stage that only exists to end the chain,
// returning back up to start_pipeline(), and from there to the caller.
#if defined(__i386__) || defined(_M_IX86) || defined(__arm__)
    extern "C" void WRAP(just_return)(Params*, void**, F,F,F,F) {}
#else
    extern "C" void WRAP(just_return)(size_t, void**, size_t,size_t, F,F,F,F, F,F,F,F) {}
#endif


// We could start defining normal Stages now.  But first, some helper functions.

// These load() and store() methods are tail-aware,
// but focus mainly on keeping the at-stride tail==0 case fast.

template <typename V, typename T>
SI V load(const T* src, size_t tail) {
#if !defined(JUMPER_IS_SCALAR)
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        V v{};  // Any inactive lanes are zeroed.
        switch (tail) {
            case 7: v[6] = src[6];
            case 6: v[5] = src[5];
            case 5: v[4] = src[4];
            case 4: memcpy(&v, src, 4*sizeof(T)); break;
            case 3: v[2] = src[2];
            case 2: memcpy(&v, src, 2*sizeof(T)); break;
            case 1: memcpy(&v, src, 1*sizeof(T)); break;
        }
        return v;
    }
#endif
    return unaligned_load<V>(src);
}

template <typename V, typename T>
SI void store(T* dst, V v, size_t tail) {
#if !defined(JUMPER_IS_SCALAR)
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        switch (tail) {
            case 7: dst[6] = v[6];
            case 6: dst[5] = v[5];
            case 5: dst[4] = v[4];
            case 4: memcpy(dst, &v, 4*sizeof(T)); break;
            case 3: dst[2] = v[2];
            case 2: memcpy(dst, &v, 2*sizeof(T)); break;
            case 1: memcpy(dst, &v, 1*sizeof(T)); break;
        }
        return;
    }
#endif
    unaligned_store(dst, v);
}

SI F from_byte(U8 b) {
    return cast(expand(b)) * (1/255.0f);
}
SI void from_565(U16 _565, F* r, F* g, F* b) {
    U32 wide = expand(_565);
    *r = cast(wide & (31<<11)) * (1.0f / (31<<11));
    *g = cast(wide & (63<< 5)) * (1.0f / (63<< 5));
    *b = cast(wide & (31<< 0)) * (1.0f / (31<< 0));
}
SI void from_4444(U16 _4444, F* r, F* g, F* b, F* a) {
    U32 wide = expand(_4444);
    *r = cast(wide & (15<<12)) * (1.0f / (15<<12));
    *g = cast(wide & (15<< 8)) * (1.0f / (15<< 8));
    *b = cast(wide & (15<< 4)) * (1.0f / (15<< 4));
    *a = cast(wide & (15<< 0)) * (1.0f / (15<< 0));
}
SI void from_8888(U32 _8888, F* r, F* g, F* b, F* a) {
    *r = cast((_8888      ) & 0xff) * (1/255.0f);
    *g = cast((_8888 >>  8) & 0xff) * (1/255.0f);
    *b = cast((_8888 >> 16) & 0xff) * (1/255.0f);
    *a = cast((_8888 >> 24)       ) * (1/255.0f);
}

// Used by load_ and store_ stages to get to the right (x,y) starting point of contiguous memory.
template <typename T>
SI T* ptr_at_xy(const SkJumper_MemoryCtx* ctx, int x, int y) {
    return (T*)ctx->pixels + y*ctx->stride + x;
}

// Used by gather_ stages to calculate the base pointer and a vector of indices to load.
template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkJumper_MemoryCtx* ctx, F x, F y) {
    *ptr = (const T*)ctx->pixels;
    return trunc_(y)*ctx->stride + trunc_(x);
}

// Now finally, normal Stages!

STAGE(seed_shader) {
    // It's important for speed to explicitly cast(x) and cast(y),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.
    float iota[] = { 0.5f,1.5f,2.5f,3.5f,4.5f,5.5f,6.5f,7.5f };
    r = cast(x) + unaligned_load<F>(iota);
    g = cast(y) + 0.5f;
    b = 1.0f;
    a = 0;
    dr = dg = db = da = 0;
}

STAGE(dither) {
    auto rate = *(const float*)ctx;

    // Get [(x,y), (x+1,y), (x+2,y), ...] loaded up in integer vectors.
    uint32_t iota[] = {0,1,2,3,4,5,6,7};
    U32 X = x + unaligned_load<U32>(iota),
        Y = y;

    // We're doing 8x8 ordered dithering, see https://en.wikipedia.org/wiki/Ordered_dithering.
    // In this case n=8 and we're using the matrix that looks like 1/64 x [ 0 48 12 60 ... ].

    // We only need X and X^Y from here on, so it's easier to just think of that as "Y".
    Y ^= X;

    // We'll mix the bottom 3 bits of each of X and Y to make 6 bits,
    // for 2^6 == 64 == 8x8 matrix values.  If X=abc and Y=def, we make fcebda.
    U32 M = (Y & 1) << 5 | (X & 1) << 4
          | (Y & 2) << 2 | (X & 2) << 1
          | (Y & 4) >> 1 | (X & 4) >> 2;

    // Scale that dither to [0,1), then (-0.5,+0.5), here using 63/128 = 0.4921875 as 0.5-epsilon.
    // We want to make sure our dither is less than 0.5 in either direction to keep exact values
    // like 0 and 1 unchanged after rounding.
    F dither = cast(M) * (2/128.0f) - (63/128.0f);

    r += rate*dither;
    g += rate*dither;
    b += rate*dither;

    r = max(0, min(r, a));
    g = max(0, min(g, a));
    b = max(0, min(b, a));
}

// load 4 floats from memory, and splat them into r,g,b,a
STAGE(uniform_color) {
    auto c = (const SkJumper_UniformColorCtx*)ctx;
    r = c->r;
    g = c->g;
    b = c->b;
    a = c->a;
}

// splats opaque-black into r,g,b,a
STAGE(black_color) {
    r = g = b = 0.0f;
    a = 1.0f;
}

STAGE(white_color) {
    r = g = b = a = 1.0f;
}

// load registers r,g,b,a from context (mirrors store_rgba)
STAGE(load_rgba) {
    auto ptr = (const float*)ctx;
    r = unaligned_load<F>(ptr + 0*kStride);
    g = unaligned_load<F>(ptr + 1*kStride);
    b = unaligned_load<F>(ptr + 2*kStride);
    a = unaligned_load<F>(ptr + 3*kStride);
}

// store registers r,g,b,a into context (mirrors load_rgba)
STAGE(store_rgba) {
    auto ptr = (float*)ctx;
    unaligned_store(ptr + 0*kStride, r);
    unaligned_store(ptr + 1*kStride, g);
    unaligned_store(ptr + 2*kStride, b);
    unaligned_store(ptr + 3*kStride, a);
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

SI F inv(F x) { return 1.0f - x; }
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
BLEND_MODE(plus_)    { return min(s + d, 1.0f); }  // We can clamp to either 1 or sa.
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
    return if_then_else(d == da,    d +    s*inv(da),
           if_then_else(s ==  0, /* s + */ d*inv(sa),
                                 sa*(da - min(da, (da-d)*sa*rcp(s))) + s*inv(da) + d*inv(sa)));
}
BLEND_MODE(colordodge) {
    return if_then_else(d ==  0, /* d + */ s*inv(da),
           if_then_else(s == sa,    s +    d*inv(sa),
                                 sa*min(da, (d*sa)*rcp(sa - s)) + s*inv(da) + d*inv(sa)));
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
    F darkSrc = d*(sa + (s2 - sa)*(1.0f - m)),     // Used in case 1.
      darkDst = (m4*m4 + m4)*(m - 1.0f) + 7.0f*m,  // Used in case 2.
      liteDst = rcp(rsqrt(m)) - m,                 // Used in case 3.
      liteSrc = d*sa + da*(s2 - sa) * if_then_else(two(two(d)) <= da, darkDst, liteDst); // 2 or 3?
    return s*inv(da) + d*inv(sa) + if_then_else(s2 <= sa, darkSrc, liteSrc);      // 1 or (2 or 3)?
}
#undef BLEND_MODE

// We're basing our implemenation of non-separable blend modes on
//   https://www.w3.org/TR/compositing-1/#blendingnonseparable.
// and
//   https://www.khronos.org/registry/OpenGL/specs/es/3.2/es_spec_3.2.pdf
// They're equivalent, but ES' math has been better simplified.
//
// Anything extra we add beyond that is to make the math work with premul inputs.

SI F max(F r, F g, F b) { return max(r, max(g, b)); }
SI F min(F r, F g, F b) { return min(r, min(g, b)); }

SI F sat(F r, F g, F b) { return max(r,g,b) - min(r,g,b); }
SI F lum(F r, F g, F b) { return r*0.30f + g*0.59f + b*0.11f; }

SI void set_sat(F* r, F* g, F* b, F s) {
    F mn  = min(*r,*g,*b),
      mx  = max(*r,*g,*b),
      sat = mx - mn;

    // Map min channel to 0, max channel to s, and scale the middle proportionally.
    auto scale = [=](F c) {
        return if_then_else(sat == 0, 0, (c - mn) * s / sat);
    };
    *r = scale(*r);
    *g = scale(*g);
    *b = scale(*b);
}
SI void set_lum(F* r, F* g, F* b, F l) {
    F diff = l - lum(*r, *g, *b);
    *r += diff;
    *g += diff;
    *b += diff;
}
SI void clip_color(F* r, F* g, F* b, F a) {
    F mn = min(*r, *g, *b),
      mx = max(*r, *g, *b),
      l  = lum(*r, *g, *b);

    auto clip = [=](F c) {
        c = if_then_else(mn >= 0, c, l + (c - l) * (    l) / (l - mn)   );
        c = if_then_else(mx >  a,    l + (c - l) * (a - l) / (mx - l), c);
        c = max(c, 0);  // Sometimes without this we may dip just a little negative.
        return c;
    };
    *r = clip(*r);
    *g = clip(*g);
    *b = clip(*b);
}

STAGE(hue) {
    F R = r*a,
      G = g*a,
      B = b*a;

    set_sat(&R, &G, &B, sat(dr,dg,db)*a);
    set_lum(&R, &G, &B, lum(dr,dg,db)*a);
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}
STAGE(saturation) {
    F R = dr*a,
      G = dg*a,
      B = db*a;

    set_sat(&R, &G, &B, sat( r, g, b)*da);
    set_lum(&R, &G, &B, lum(dr,dg,db)* a);  // (This is not redundant.)
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}
STAGE(color) {
    F R = r*da,
      G = g*da,
      B = b*da;

    set_lum(&R, &G, &B, lum(dr,dg,db)*a);
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}
STAGE(luminosity) {
    F R = dr*a,
      G = dg*a,
      B = db*a;

    set_lum(&R, &G, &B, lum(r,g,b)*da);
    clip_color(&R,&G,&B, a*da);

    r = r*inv(da) + dr*inv(a) + R;
    g = g*inv(da) + dg*inv(a) + G;
    b = b*inv(da) + db*inv(a) + B;
    a = a + da - a*da;
}

STAGE(srcover_rgba_8888) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

    U32 dst = load<U32>(ptr, tail);
    dr = cast((dst      ) & 0xff);
    dg = cast((dst >>  8) & 0xff);
    db = cast((dst >> 16) & 0xff);
    da = cast((dst >> 24)       );
    // {dr,dg,db,da} are in [0,255]
    // { r, g, b, a} are in [0,  1]

    r = mad(dr, inv(a), r*255.0f);
    g = mad(dg, inv(a), g*255.0f);
    b = mad(db, inv(a), b*255.0f);
    a = mad(da, inv(a), a*255.0f);
    // { r, g, b, a} are now in [0,255]

    dst = round(r, 1.0f)
        | round(g, 1.0f) <<  8
        | round(b, 1.0f) << 16
        | round(a, 1.0f) << 24;
    store(ptr, dst, tail);
}

STAGE(clamp_0) {
    r = max(r, 0);
    g = max(g, 0);
    b = max(b, 0);
    a = max(a, 0);
}

STAGE(clamp_1) {
    r = min(r, 1.0f);
    g = min(g, 1.0f);
    b = min(b, 1.0f);
    a = min(a, 1.0f);
}

STAGE(clamp_a) {
    a = min(a, 1.0f);
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}

STAGE(clamp_a_dst) {
    da = min(da, 1.0f);
    dr = min(dr, da);
    dg = min(dg, da);
    db = min(db, da);
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
STAGE(invert) {
    r = inv(r);
    g = inv(g);
    b = inv(b);
    a = inv(a);
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
STAGE(premul_dst) {
    dr = dr * da;
    dg = dg * da;
    db = db * da;
}
STAGE(unpremul) {
    auto scale = if_then_else(a == 0, 0, 1.0f / a);
    r *= scale;
    g *= scale;
    b *= scale;
}

SI F from_srgb(F s) {
    auto lo = s * (1/12.92f);
    auto hi = mad(s*s, mad(s, 0.3000f, 0.6975f), 0.0025f);
    return if_then_else(s < 0.055f, lo, hi);
}

STAGE(from_srgb) {
    r = from_srgb(r);
    g = from_srgb(g);
    b = from_srgb(b);
}
STAGE(from_srgb_dst) {
    dr = from_srgb(dr);
    dg = from_srgb(dg);
    db = from_srgb(db);
}
STAGE(to_srgb) {
    auto fn = [&](F l) {
        // We tweak c and d for each instruction set to make sure fn(1) is exactly 1.
    #if defined(JUMPER_IS_SSE2) || defined(JUMPER_IS_SSE41) || \
        defined(JUMPER_IS_AVX ) || defined(JUMPER_IS_AVX2 )
        const float c = 1.130048394203f,
                    d = 0.141357362270f;
    #elif defined(JUMPER_IS_NEON)
        const float c = 1.129999995232f,
                    d = 0.141381442547f;
    #else
        const float c = 1.129999995232f,
                    d = 0.141377761960f;
    #endif
        F t = rsqrt(l);
        auto lo = l * 12.92f;
        auto hi = mad(t, mad(t, -0.0024542345f, 0.013832027f), c)
                * rcp(d + t);
        return if_then_else(l < 0.00465985f, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(rgb_to_hsl) {
    F mx = max(r,g,b),
      mn = min(r,g,b),
      d = mx - mn,
      d_rcp = 1.0f / d;

    F h = (1/6.0f) *
          if_then_else(mx == mn, 0,
          if_then_else(mx ==  r, (g-b)*d_rcp + if_then_else(g < b, 6.0f, 0),
          if_then_else(mx ==  g, (b-r)*d_rcp + 2.0f,
                                 (r-g)*d_rcp + 4.0f)));

    F l = (mx + mn) * 0.5f;
    F s = if_then_else(mx == mn, 0,
                       d / if_then_else(l > 0.5f, 2.0f-mx-mn, mx+mn));

    r = h;
    g = s;
    b = l;
}
STAGE(hsl_to_rgb) {
    F h = r,
      s = g,
      l = b;

    F q = l + if_then_else(l >= 0.5f, s - l*s, l*s),
      p = 2.0f*l - q;

    auto hue_to_rgb = [&](F t) {
        t = fract(t);

        F r = p;
        r = if_then_else(t >= 4/6.0f, r, p + (q-p)*(4.0f - 6.0f*t));
        r = if_then_else(t >= 3/6.0f, r, q);
        r = if_then_else(t >= 1/6.0f, r, p + (q-p)*(       6.0f*t));
        return r;
    };

    r = if_then_else(s == 0, l, hue_to_rgb(h + (1/3.0f)));
    g = if_then_else(s == 0, l, hue_to_rgb(h           ));
    b = if_then_else(s == 0, l, hue_to_rgb(h - (1/3.0f)));
}

// Derive alpha's coverage from rgb coverage and the values of src and dst alpha.
SI F alpha_coverage_from_rgb_coverage(F a, F da, F cr, F cg, F cb) {
    return if_then_else(a < da, min(cr,cg,cb)
                              , max(cr,cg,cb));
}

STAGE(scale_1_float) {
    auto c = *(const float*)ctx;

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_u8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_565) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

    F ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = r * cr;
    g = g * cg;
    b = b * cb;
    a = a * ca;
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
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    auto scales = load<U8>(ptr, tail);
    auto c = from_byte(scales);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_565) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

    F ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

STAGE(load_tables) {
    auto c = (const SkJumper_LoadTablesCtx*)ctx;

    auto px = load<U32>((const uint32_t*)c->src + x, tail);
    r = gather(c->r, (px      ) & 0xff);
    g = gather(c->g, (px >>  8) & 0xff);
    b = gather(c->b, (px >> 16) & 0xff);
    a = cast(        (px >> 24)) * (1/255.0f);
}
STAGE(load_tables_u16_be) {
    auto c = (const SkJumper_LoadTablesCtx*)ctx;
    auto ptr = (const uint16_t*)c->src + 4*x;

    U16 R,G,B,A;
    load4(ptr, tail, &R,&G,&B,&A);

    // c->src is big-endian, so & 0xff grabs the 8 most signficant bits.
    r = gather(c->r, expand(R) & 0xff);
    g = gather(c->g, expand(G) & 0xff);
    b = gather(c->b, expand(B) & 0xff);
    a = (1/65535.0f) * cast(expand(bswap(A)));
}
STAGE(load_tables_rgb_u16_be) {
    auto c = (const SkJumper_LoadTablesCtx*)ctx;
    auto ptr = (const uint16_t*)c->src + 3*x;

    U16 R,G,B;
    load3(ptr, tail, &R,&G,&B);

    // c->src is big-endian, so & 0xff grabs the 8 most signficant bits.
    r = gather(c->r, expand(R) & 0xff);
    g = gather(c->g, expand(G) & 0xff);
    b = gather(c->b, expand(B) & 0xff);
    a = 1.0f;
}

STAGE(byte_tables) {
    struct Tables { const uint8_t *r, *g, *b, *a; };
    auto tables = (const Tables*)ctx;

    r = from_byte(gather(tables->r, round(r, 255.0f)));
    g = from_byte(gather(tables->g, round(g, 255.0f)));
    b = from_byte(gather(tables->b, round(b, 255.0f)));
    a = from_byte(gather(tables->a, round(a, 255.0f)));
}

STAGE(byte_tables_rgb) {
    struct Tables { const uint8_t *r, *g, *b; int n; };
    auto tables = (const Tables*)ctx;

    F scale = tables->n - 1;
    r = from_byte(gather(tables->r, round(r, scale)));
    g = from_byte(gather(tables->g, round(g, scale)));
    b = from_byte(gather(tables->b, round(b, scale)));
}

SI F table(F v, const SkJumper_TableCtx* ctx) {
    return gather(ctx->table, round(v, ctx->size - 1));
}
STAGE(table_r) { r = table(r, ctx); }
STAGE(table_g) { g = table(g, ctx); }
STAGE(table_b) { b = table(b, ctx); }
STAGE(table_a) { a = table(a, ctx); }

SI F parametric(F v, const SkJumper_ParametricTransferFunction* ctx) {
    F r = if_then_else(v <= ctx->D, mad(ctx->C, v, ctx->F)
                                  , approx_powf(mad(ctx->A, v, ctx->B), ctx->G) + ctx->E);
    return min(max(r, 0), 1.0f);  // Clamp to [0,1], with argument order mattering to handle NaN.
}
STAGE(parametric_r) { r = parametric(r, ctx); }
STAGE(parametric_g) { g = parametric(g, ctx); }
STAGE(parametric_b) { b = parametric(b, ctx); }
STAGE(parametric_a) { a = parametric(a, ctx); }

STAGE(gamma) {
    float G = *(const float*)ctx;
    r = approx_powf(r,G);
    g = approx_powf(g,G);
    b = approx_powf(b,G);
}

STAGE(lab_to_xyz) {
    F L = r * 100.0f,
      A = g * 255.0f - 128.0f,
      B = b * 255.0f - 128.0f;

    F Y = (L + 16.0f) * (1/116.0f),
      X = Y + A*(1/500.0f),
      Z = Y - B*(1/200.0f);

    X = if_then_else(X*X*X > 0.008856f, X*X*X, (X - (16/116.0f)) * (1/7.787f));
    Y = if_then_else(Y*Y*Y > 0.008856f, Y*Y*Y, (Y - (16/116.0f)) * (1/7.787f));
    Z = if_then_else(Z*Z*Z > 0.008856f, Z*Z*Z, (Z - (16/116.0f)) * (1/7.787f));

    // Adjust to D50 illuminant.
    r = X * 0.96422f;
    g = Y           ;
    b = Z * 0.82521f;
}

STAGE(load_a8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    r = g = b = 0.0f;
    a = from_byte(load<U8>(ptr, tail));
}
STAGE(load_a8_dst) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    dr = dg = db = 0.0f;
    da = from_byte(load<U8>(ptr, tail));
}
STAGE(gather_a8) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = 0.0f;
    a = from_byte(gather(ptr, ix));
}
STAGE(store_a8) {
    auto ptr = ptr_at_xy<uint8_t>(ctx, x,y);

    U8 packed = pack(pack(round(a, 255.0f)));
    store(ptr, packed, tail);
}

STAGE(load_g8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    r = g = b = from_byte(load<U8>(ptr, tail));
    a = 1.0f;
}
STAGE(load_g8_dst) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    dr = dg = db = from_byte(load<U8>(ptr, tail));
    da = 1.0f;
}
STAGE(gather_g8) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    r = g = b = from_byte(gather(ptr, ix));
    a = 1.0f;
}

STAGE(load_565) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);

    from_565(load<U16>(ptr, tail), &r,&g,&b);
    a = 1.0f;
}
STAGE(load_565_dst) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);

    from_565(load<U16>(ptr, tail), &dr,&dg,&db);
    da = 1.0f;
}
STAGE(gather_565) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_565(gather(ptr, ix), &r,&g,&b);
    a = 1.0f;
}
STAGE(store_565) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, x,y);

    U16 px = pack( round(r, 31.0f) << 11
                 | round(g, 63.0f) <<  5
                 | round(b, 31.0f)      );
    store(ptr, px, tail);
}

STAGE(load_4444) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);
    from_4444(load<U16>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_4444_dst) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);
    from_4444(load<U16>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(gather_4444) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_4444(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_4444) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, x,y);
    U16 px = pack( round(r, 15.0f) << 12
                 | round(g, 15.0f) <<  8
                 | round(b, 15.0f) <<  4
                 | round(a, 15.0f)      );
    store(ptr, px, tail);
}

STAGE(load_8888) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    from_8888(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_8888_dst) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    from_8888(load<U32>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(gather_8888) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_8888(gather(ptr, ix), &r,&g,&b,&a);
}
STAGE(store_8888) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

    U32 px = round(r, 255.0f)
           | round(g, 255.0f) <<  8
           | round(b, 255.0f) << 16
           | round(a, 255.0f) << 24;
    store(ptr, px, tail);
}

STAGE(load_bgra) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    from_8888(load<U32>(ptr, tail), &b,&g,&r,&a);
}
STAGE(load_bgra_dst) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    from_8888(load<U32>(ptr, tail), &db,&dg,&dr,&da);
}
STAGE(gather_bgra) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, r,g);
    from_8888(gather(ptr, ix), &b,&g,&r,&a);
}
STAGE(store_bgra) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

    U32 px = round(b, 255.0f)
           | round(g, 255.0f) <<  8
           | round(r, 255.0f) << 16
           | round(a, 255.0f) << 24;
    store(ptr, px, tail);
}

STAGE(load_f16) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, x,y);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
    r = from_half(R);
    g = from_half(G);
    b = from_half(B);
    a = from_half(A);
}
STAGE(load_f16_dst) {
    auto ptr = ptr_at_xy<const uint64_t>(ctx, x,y);

    U16 R,G,B,A;
    load4((const uint16_t*)ptr,tail, &R,&G,&B,&A);
    dr = from_half(R);
    dg = from_half(G);
    db = from_half(B);
    da = from_half(A);
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
    auto ptr = ptr_at_xy<uint64_t>(ctx, x,y);
    store4((uint16_t*)ptr,tail, to_half(r)
                              , to_half(g)
                              , to_half(b)
                              , to_half(a));
}

STAGE(load_u16_be) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, 4*x,y);

    U16 R,G,B,A;
    load4(ptr,tail, &R,&G,&B,&A);

    r = (1/65535.0f) * cast(expand(bswap(R)));
    g = (1/65535.0f) * cast(expand(bswap(G)));
    b = (1/65535.0f) * cast(expand(bswap(B)));
    a = (1/65535.0f) * cast(expand(bswap(A)));
}
STAGE(load_rgb_u16_be) {
    auto ptr = ptr_at_xy<const uint16_t>(ctx, 3*x,y);

    U16 R,G,B;
    load3(ptr,tail, &R,&G,&B);

    r = (1/65535.0f) * cast(expand(bswap(R)));
    g = (1/65535.0f) * cast(expand(bswap(G)));
    b = (1/65535.0f) * cast(expand(bswap(B)));
    a = 1.0f;
}
STAGE(store_u16_be) {
    auto ptr = ptr_at_xy<uint16_t>(ctx, 4*x,y);

    U16 R = bswap(pack(round(r, 65535.0f))),
        G = bswap(pack(round(g, 65535.0f))),
        B = bswap(pack(round(b, 65535.0f))),
        A = bswap(pack(round(a, 65535.0f)));

    store4(ptr,tail, R,G,B,A);
}

STAGE(load_f32) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*x,y);
    load4(ptr,tail, &r,&g,&b,&a);
}
STAGE(load_f32_dst) {
    auto ptr = ptr_at_xy<const float>(ctx, 4*x,y);
    load4(ptr,tail, &dr,&dg,&db,&da);
}
STAGE(store_f32) {
    auto ptr = ptr_at_xy<float>(ctx, 4*x,y);
    store4(ptr,tail, r,g,b,a);
}

SI F ulp_before(F f) {
    U32 bits = -1 + unaligned_load<U32>(&f);
    return unaligned_load<F>(&bits);
}

// We make sure to funnel all three tilers through exclusive_clamp() so that we're guaranteed
// to be in [0,ctx->scale), even in the presence of bugs or floating point precision issues.
SI F exclusive_clamp(F v, const SkJumper_TileCtx* ctx) {
    v = max(0,v);
    return min(v, ulp_before(ctx->scale));
}
SI F exclusive_repeat(F v, const SkJumper_TileCtx* ctx) {
    v = v - floor_(v*ctx->invScale)*ctx->scale;
    return exclusive_clamp(v, ctx);
}
SI F exclusive_mirror(F v, const SkJumper_TileCtx* ctx) {
    auto limit = ctx->scale;
    auto invLimit = ctx->invScale;
    v = abs_( (v-limit) - (limit+limit)*floor_((v-limit)*(invLimit*0.5f)) - limit );
    return exclusive_clamp(v, ctx);
}
// Clamp x or y to [0,limit) == [0,limit - 1 ulp] (think, sampling from images).
STAGE(clamp_x)  { r = exclusive_clamp (r, (const SkJumper_TileCtx*)ctx); }
STAGE(clamp_y)  { g = exclusive_clamp (g, (const SkJumper_TileCtx*)ctx); }
STAGE(repeat_x) { r = exclusive_repeat(r, (const SkJumper_TileCtx*)ctx); }
STAGE(repeat_y) { g = exclusive_repeat(g, (const SkJumper_TileCtx*)ctx); }
STAGE(mirror_x) { r = exclusive_mirror(r, (const SkJumper_TileCtx*)ctx); }
STAGE(mirror_y) { g = exclusive_mirror(g, (const SkJumper_TileCtx*)ctx); }

// Clamp x to [0,1], both sides exclusive (think, gradients).
STAGE( clamp_x_1) { r = min(max(0, r), 1.0f); }
STAGE(repeat_x_1) { r = r - floor_(r); }
STAGE(mirror_x_1) { r = abs_( (r-1.0f) - two(floor_((r-1.0f)*0.5f)) - 1.0f ); }

STAGE(luminance_to_alpha) {
    a = r*0.2126f + g*0.7152f + b*0.0722f;
    r = g = b = 0;
}

STAGE(matrix_translate) {
    auto m = (const float*)ctx;

    r += m[0];
    g += m[1];
}
STAGE(matrix_scale_translate) {
    auto m = (const float*)ctx;

    r = mad(r,m[2], m[0]);
    g = mad(g,m[3], m[1]);
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
STAGE(matrix_4x3) {
    auto m = (const float*)ctx;
    auto X = r,
         Y = g;

    r = mad(X, m[0], mad(Y, m[4], m[ 8]));
    g = mad(X, m[1], mad(Y, m[5], m[ 9]));
    b = mad(X, m[2], mad(Y, m[6], m[10]));
    a = mad(X, m[3], mad(Y, m[7], m[11]));
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

SI void gradient_lookup(const SkJumper_GradientCtx* c, U32 idx, F t,
                        F* r, F* g, F* b, F* a) {
    F fr, br, fg, bg, fb, bb, fa, ba;
#if defined(JUMPER_IS_AVX2)
    if (c->stopCount <=8) {
        fr = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), idx);
        br = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), idx);
        fg = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), idx);
        bg = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), idx);
        fb = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), idx);
        bb = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), idx);
        fa = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), idx);
        ba = _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), idx);
    } else
#endif
    {
        fr = gather(c->fs[0], idx);
        br = gather(c->bs[0], idx);
        fg = gather(c->fs[1], idx);
        bg = gather(c->bs[1], idx);
        fb = gather(c->fs[2], idx);
        bb = gather(c->bs[2], idx);
        fa = gather(c->fs[3], idx);
        ba = gather(c->bs[3], idx);
    }

    *r = mad(t, fr, br);
    *g = mad(t, fg, bg);
    *b = mad(t, fb, bb);
    *a = mad(t, fa, ba);
}

STAGE(evenly_spaced_gradient) {
    auto c = (const SkJumper_GradientCtx*)ctx;
    auto t = r;
    auto idx = trunc_(t * (c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(gauss_a_to_rgba) {
    // x = 1 - x;
    // exp(-x * x * 4) - 0.018f;
    // ... now approximate with quartic
    //
    const float c4 = -2.26661229133605957031f;
    const float c3 = 2.89795351028442382812f;
    const float c2 = 0.21345567703247070312f;
    const float c1 = 0.15489584207534790039f;
    const float c0 = 0.00030726194381713867f;
    a = mad(a, mad(a, mad(a, mad(a, c4, c3), c2), c1), c0);
    r = a;
    g = a;
    b = a;
}

STAGE(gradient) {
    auto c = (const SkJumper_GradientCtx*)ctx;
    auto t = r;
    U32 idx = 0;

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += if_then_else(t >= c->ts[i], U32(1), U32(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE(evenly_spaced_2_stop_gradient) {
    struct Ctx { float f[4], b[4]; };
    auto c = (const Ctx*)ctx;

    auto t = r;
    r = mad(t, c->f[0], c->b[0]);
    g = mad(t, c->f[1], c->b[1]);
    b = mad(t, c->f[2], c->b[2]);
    a = mad(t, c->f[3], c->b[3]);
}

STAGE(xy_to_unit_angle) {
    F X = r,
      Y = g;
    F xabs = abs_(X),
      yabs = abs_(Y);

    F slope = min(xabs, yabs)/max(xabs, yabs);
    F s = slope * slope;

    // Use a 7th degree polynomial to approximate atan.
    // This was generated using sollya.gforge.inria.fr.
    // A float optimized polynomial was generated using the following command.
    // P1 = fpminimax((1/(2*Pi))*atan(x),[|1,3,5,7|],[|24...|],[2^(-40),1],relative);
    F phi = slope
             * (0.15912117063999176025390625f     + s
             * (-5.185396969318389892578125e-2f   + s
             * (2.476101927459239959716796875e-2f + s
             * (-7.0547382347285747528076171875e-3f))));

    phi = if_then_else(xabs < yabs, 1.0f/4.0f - phi, phi);
    phi = if_then_else(X < 0.0f   , 1.0f/2.0f - phi, phi);
    phi = if_then_else(Y < 0.0f   , 1.0f - phi     , phi);
    phi = if_then_else(phi != phi , 0              , phi);  // Check for NaN.
    r = phi;
}

STAGE(xy_to_radius) {
    F X2 = r * r,
      Y2 = g * g;
    r = sqrt_(X2 + Y2);
}

SI F solve_2pt_conical_quadratic(const SkJumper_2PtConicalCtx* c, F x, F y, F (*select)(F, F)) {
    // At this point, (x, y) is mapped into a synthetic gradient space with
    // the start circle centerd on (0, 0), and the end circle centered on (1, 0)
    // (see the stage setup).
    //
    // We're searching along X-axis for x', such that
    //
    //   1) r(x') is a linear interpolation between r0 and r1
    //   2) (x, y) is on the circle C(x', 0, r(x'))
    //
    // Solving this system boils down to a quadratic equation with coefficients
    //
    //   a = 1 - (r1 - r0)^2             <- constant, precomputed in ctx->fCoeffA)
    //
    //   b = -2 * (x + (r1 - r0) * r0)
    //
    //   c = x^2 + y^2 - r0^2
    //
    // Since the start/end circle centers are the extremes of the [0, 1] interval
    // on the X axis, the solution (x') is exactly the t we are looking for.

    const F coeffA = c->fCoeffA,
            coeffB = -2 * (x + c->fDR*c->fR0),
            coeffC = x*x + y*y - c->fR0*c->fR0;

    const F disc      = mad(coeffB, coeffB, -4 * coeffA * coeffC);
    const F sqrt_disc = sqrt_(disc);

    const F invCoeffA = c->fInvCoeffA;
    return select((-coeffB + sqrt_disc) * (invCoeffA * 0.5f),
                  (-coeffB - sqrt_disc) * (invCoeffA * 0.5f));
}

STAGE(xy_to_2pt_conical_quadratic_max) {
    r = solve_2pt_conical_quadratic(ctx, r, g, max);
}

STAGE(xy_to_2pt_conical_quadratic_min) {
    r = solve_2pt_conical_quadratic(ctx, r, g, min);
}

STAGE(xy_to_2pt_conical_linear) {
    auto* c = (const SkJumper_2PtConicalCtx*)ctx;

    const F coeffB = -2 * (r + c->fDR*c->fR0),
            coeffC = r*r + g*g - c->fR0*c->fR0;

    r = -coeffC / coeffB;
}

STAGE(mask_2pt_conical_degenerates) {
    auto* c = (SkJumper_2PtConicalCtx*)ctx;

    // Compute and save a mask for degenerate values.
    U32 mask = 0xffffffff;

    // TODO: mtklein kindly volunteered to revisit this at some point.
#if defined(JUMPER_IS_SCALAR)
    // The portable version is more involved, 'cause we only get one bit back.
    mask = mask & if_then_else(mad(r, c->fDR, c->fR0) >= 0, U32(0xffffffff), U32(0)); // R(t) >= 0
    mask = mask & if_then_else(r == r,                      U32(0xffffffff), U32(0)); // t != NaN
#else
    // Vector comparisons set all bits, so we can use something like this.
    mask = mask & (mad(r, c->fDR, c->fR0) >= 0);  // R(t) >= 0
    mask = mask & (r == r);                       // t != NaN
#endif

    unaligned_store(&c->fMask, mask);
}

STAGE(apply_vector_mask) {
    const U32 mask = unaligned_load<U32>((const uint32_t*)ctx);
    r = bit_cast<F>(bit_cast<U32>(r) & mask);
    g = bit_cast<F>(bit_cast<U32>(g) & mask);
    b = bit_cast<F>(bit_cast<U32>(b) & mask);
    a = bit_cast<F>(bit_cast<U32>(a) & mask);
}

STAGE(save_xy) {
    auto c = (SkJumper_SamplerCtx*)ctx;

    // Whether bilinear or bicubic, all sample points are at the same fractional offset (fx,fy).
    // They're either the 4 corners of a logical 1x1 pixel or the 16 corners of a 3x3 grid
    // surrounding (x,y) at (0.5,0.5) off-center.
    F fx = fract(r + 0.5f),
      fy = fract(g + 0.5f);

    // Samplers will need to load x and fx, or y and fy.
    unaligned_store(c->x,  r);
    unaligned_store(c->y,  g);
    unaligned_store(c->fx, fx);
    unaligned_store(c->fy, fy);
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
    *x = unaligned_load<F>(ctx->x) + (kScale * 0.5f);
    F fx = unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -1) { scalex = 1.0f - fx; }
    if (kScale == +1) { scalex =        fx; }
    unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bilinear_y(SkJumper_SamplerCtx* ctx, F* y) {
    *y = unaligned_load<F>(ctx->y) + (kScale * 0.5f);
    F fy = unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -1) { scaley = 1.0f - fy; }
    if (kScale == +1) { scaley =        fy; }
    unaligned_store(ctx->scaley, scaley);
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
    return mad(t, mad(t, mad((-21/18.0f), t, (27/18.0f)), (9/18.0f)), (1/18.0f));
}
SI F bicubic_far(F t) {
    // 0/18 + 0/18*t - 6/18t^2 + 7/18t^3 == t^2 (7/18t - 6/18)
    return (t*t)*mad((7/18.0f), t, (-6/18.0f));
}

template <int kScale>
SI void bicubic_x(SkJumper_SamplerCtx* ctx, F* x) {
    *x = unaligned_load<F>(ctx->x) + (kScale * 0.5f);
    F fx = unaligned_load<F>(ctx->fx);

    F scalex;
    if (kScale == -3) { scalex = bicubic_far (1.0f - fx); }
    if (kScale == -1) { scalex = bicubic_near(1.0f - fx); }
    if (kScale == +1) { scalex = bicubic_near(       fx); }
    if (kScale == +3) { scalex = bicubic_far (       fx); }
    unaligned_store(ctx->scalex, scalex);
}
template <int kScale>
SI void bicubic_y(SkJumper_SamplerCtx* ctx, F* y) {
    *y = unaligned_load<F>(ctx->y) + (kScale * 0.5f);
    F fy = unaligned_load<F>(ctx->fy);

    F scaley;
    if (kScale == -3) { scaley = bicubic_far (1.0f - fy); }
    if (kScale == -1) { scaley = bicubic_near(1.0f - fy); }
    if (kScale == +1) { scaley = bicubic_near(       fy); }
    if (kScale == +3) { scaley = bicubic_far (       fy); }
    unaligned_store(ctx->scaley, scaley);
}

STAGE(bicubic_n3x) { bicubic_x<-3>(ctx, &r); }
STAGE(bicubic_n1x) { bicubic_x<-1>(ctx, &r); }
STAGE(bicubic_p1x) { bicubic_x<+1>(ctx, &r); }
STAGE(bicubic_p3x) { bicubic_x<+3>(ctx, &r); }

STAGE(bicubic_n3y) { bicubic_y<-3>(ctx, &g); }
STAGE(bicubic_n1y) { bicubic_y<-1>(ctx, &g); }
STAGE(bicubic_p1y) { bicubic_y<+1>(ctx, &g); }
STAGE(bicubic_p3y) { bicubic_y<+3>(ctx, &g); }

STAGE(callback) {
    auto c = (SkJumper_CallbackCtx*)ctx;
    store4(c->rgba,0, r,g,b,a);
    c->fn(c, tail ? tail : kStride);
    load4(c->read_from,0, &r,&g,&b,&a);
}

// Our general strategy is to recursively interpolate each dimension,
// accumulating the index to sample at, and our current pixel stride to help accumulate the index.
template <int dim>
SI void color_lookup_table(const SkJumper_ColorLookupTableCtx* ctx,
                           F& r, F& g, F& b, F a, U32 index, U32 stride) {
    // We'd logically like to sample this dimension at x.
    int limit = ctx->limits[dim-1];
    F src;
    switch(dim) {
        case 1: src = r; break;
        case 2: src = g; break;
        case 3: src = b; break;
        case 4: src = a; break;
    }
    F x = src * (limit - 1);

    // We can't index an array by a float (darn) so we have to snap to nearby integers lo and hi.
    U32 lo = trunc_(x          ),
        hi = trunc_(x + 0.9999f);

    // Recursively sample at lo and hi.
    F lr = r, lg = g, lb = b,
      hr = r, hg = g, hb = b;
    color_lookup_table<dim-1>(ctx, lr,lg,lb,a, stride*lo + index, stride*limit);
    color_lookup_table<dim-1>(ctx, hr,hg,hb,a, stride*hi + index, stride*limit);

    // Linearly interpolate those colors based on their distance to x.
    F t = x - cast(lo);
    r = lerp(lr, hr, t);
    g = lerp(lg, hg, t);
    b = lerp(lb, hb, t);
}

// Bottom out our recursion at 0 dimensions, i.e. just return the colors at index.
template<>
inline void color_lookup_table<0>(const SkJumper_ColorLookupTableCtx* ctx,
                                  F& r, F& g, F& b, F a, U32 index, U32 stride) {
    r = gather(ctx->table, 3*index+0);
    g = gather(ctx->table, 3*index+1);
    b = gather(ctx->table, 3*index+2);
}

STAGE(clut_3D) {
    color_lookup_table<3>(ctx, r,g,b,a, 0,1);
    // This 3D color lookup table leaves alpha alone.
}
STAGE(clut_4D) {
    color_lookup_table<4>(ctx, r,g,b,a, 0,1);
    // "a" was really CMYK's K, so we just set alpha opaque.
    a = 1.0f;
}
