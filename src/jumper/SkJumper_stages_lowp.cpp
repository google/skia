/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This restricted SkJumper backend works on 8-bit per channel pixels stored in
// 16-bit channels.  This is a last attempt to write a performant low-precision
// backend with stage definitions that can be shared by x86 and ARM.

#include "SkJumper.h"

#define SI static inline

#if defined(__ARM_NEON)
    #include <arm_neon.h>

    #if defined(__arm__)
        #define ABI __attribute__((pcs("aapcs-vfp")))
    #else
        #define ABI
    #endif

    #define N 8
    #define WRAP(name) sk_##name##_8bit
#else
    #include <immintrin.h>

    #define ABI

    #if defined(__AVX2__)
        #define N 16
        #define WRAP(name) sk_##name##_hsw_8bit
    #elif defined(__SSE4_1__)
        #define N 8
        #define WRAP(name) sk_##name##_sse41_8bit
    #elif defined(__SSE2__)
        #define N 8
        #define WRAP(name) sk_##name##_sse2_8bit
    #endif
#endif

using U8  = uint8_t  __attribute__((ext_vector_type(N)));
using U16 = uint16_t __attribute__((ext_vector_type(N)));
using U32 = uint32_t __attribute__((ext_vector_type(N)));

// We pass program as the second argument so that load_and_inc() will find it in %rsi on x86-64.
using Stage = void (ABI*)(size_t tail, void** program, size_t x, size_t y,
                          U16  r, U16  g, U16  b, U16  a,
                          U16 dr, U16 dg, U16 db, U16 da);

SI void* load_and_inc(void**& program) {
#if defined(__x86_64__)
    // If program is in %rsi, this is a single-instruction *program++.
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));
    return rax;
#else
    return *program++;
#endif
}

MAYBE_MSABI
ABI extern "C" void WRAP(start_pipeline)(const size_t x0,
                                         const size_t y0,
                                         const size_t xlimit,
                                         const size_t ylimit,
                                         void** program) {
    auto start = (Stage)load_and_inc(program);
    for (size_t y = y0; y < ylimit; y++) {
        size_t x = x0;
        for (; x + N <= xlimit; x += N) {
            start(   0,program,x,y, 0,0,0,0, 0,0,0,0);
        }
        if (size_t tail = xlimit - x) {
            start(tail,program,x,y, 0,0,0,0, 0,0,0,0);
        }
    }
}

ABI extern "C" void WRAP(just_return)(size_t,void**,size_t,size_t,
                                      U16,U16,U16,U16, U16,U16,U16,U16) {}

// Lazily resolved on first cast.  Does nothing if cast to Ctx::None.
struct Ctx {
    using None = decltype(nullptr);

    void*   ptr;
    void**& program;

    explicit Ctx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }
    operator None() { return nullptr; }
};

#define STAGE(name, ...)                                                               \
    SI void name##_k(__VA_ARGS__, size_t x, size_t y, size_t tail,                     \
                     U16&  r, U16&  g, U16&  b, U16&  a,                               \
                     U16& dr, U16& dg, U16& db, U16& da);                              \
    ABI extern "C" void WRAP(name)(size_t tail, void** program, size_t x, size_t y,    \
                                   U16  r, U16  g, U16  b, U16  a,                     \
                                   U16 dr, U16 dg, U16 db, U16 da) {                   \
        name##_k(Ctx{program}, x,y,tail, r,g,b,a, dr,dg,db,da);                        \
        auto next = (Stage)load_and_inc(program);                                      \
        next(tail,program,x,y, r,g,b,a, dr,dg,db,da);                                  \
    }                                                                                  \
    SI void name##_k(__VA_ARGS__, size_t x, size_t y, size_t tail,                     \
                     U16&  r, U16&  g, U16&  b, U16&  a,                               \
                     U16& dr, U16& dg, U16& db, U16& da)

SI U16 div255(U16 v) {
#if 0
    return (v+127)/255;  // The ideal rounding divide by 255.
#else
    return (v+255)/256;  // A good approximation of (v+127)/255.
#endif
}

SI U16 inv(U16 v) { return 255-v; }

template <typename Mask>
SI U16 if_then_else(Mask c, U16 t, U16 e) { return (t & c) | (e & ~c); }

SI U16 max(U16 x, U16 y) { return if_then_else(x < y, y, x); }
SI U16 min(U16 x, U16 y) { return if_then_else(x < y, x, y); }

STAGE(uniform_color, const SkJumper_UniformColorCtx* c) {
    auto rgba = (const uint8_t*)&c->rgba;
    r = rgba[0];
    g = rgba[1];
    b = rgba[2];
    a = rgba[3];
}

STAGE(set_rgb, const float rgb[3]) {
    r = rgb[0] * 255.0f + 0.5f;
    g = rgb[1] * 255.0f + 0.5f;
    b = rgb[2] * 255.0f + 0.5f;
}

STAGE(premul, Ctx::None) {
    r = div255(r * a);
    g = div255(g * a);
    b = div255(b * a);
}

STAGE(swap_rb, Ctx::None) {
    auto tmp = r;
    r = b;
    b = tmp;
}

STAGE(move_src_dst, Ctx::None) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}

STAGE(move_dst_src, Ctx::None) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}

STAGE(invert, Ctx::None) {
    r = inv(r);
    g = inv(g);
    b = inv(b);
    a = inv(a);
}

STAGE(clear,       Ctx::None) { r = g = b =   0; a =   0; }
STAGE(black_color, Ctx::None) { r = g = b =   0; a = 255; }
STAGE(white_color, Ctx::None) { r = g = b = 255; a = 255; }

// The same logic applied to all 4 channels.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE(name, Ctx::None) {                             \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = name##_channel(a,da,a,da);                   \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

    BLEND_MODE(srcatop)  { return div255( s*da + d*inv(sa) ); }
    BLEND_MODE(dstatop)  { return div255( d*sa + s*inv(da) ); }
    BLEND_MODE(srcin)    { return div255( s*da ); }
    BLEND_MODE(dstin)    { return div255( d*sa ); }
    BLEND_MODE(srcout)   { return div255( s*inv(da) ); }
    BLEND_MODE(dstout)   { return div255( d*inv(sa) ); }
    BLEND_MODE(srcover)  { return s + div255( d*inv(sa) ); }
    BLEND_MODE(dstover)  { return d + div255( s*inv(da) ); }
    BLEND_MODE(modulate) { return div255( s*d ); }
    BLEND_MODE(multiply) { return div255( s*inv(da) + d*inv(sa) + s*d ); }
    BLEND_MODE(plus_)    { return max(s+d, 255); }
    BLEND_MODE(screen)   { return s + d - div255( s*d ); }
    BLEND_MODE(xor_)     { return div255( s*inv(da) + d*inv(sa) ); }
#undef BLEND_MODE

// The same logic applied to color, and srcover for alpha.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE(name, Ctx::None) {                             \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = a + div255( da*inv(a) );                     \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

    BLEND_MODE(darken)     { return s + d -   div255( max(s*da, d*sa) ); }
    BLEND_MODE(lighten)    { return s + d -   div255( min(s*da, d*sa) ); }
    BLEND_MODE(difference) { return s + d - 2*div255( min(s*da, d*sa) ); }
    BLEND_MODE(exclusion)  { return s + d - 2*div255( s*d ); }

    BLEND_MODE(hardlight) {
        return div255( s*inv(da) + d*inv(sa) +
                       if_then_else(2*s <= sa, 2*s*d, sa*da - 2*(sa-s)*(da-d)) );
    }
    BLEND_MODE(overlay) {
        return div255( s*inv(da) + d*inv(sa) +
                       if_then_else(2*d <= da, 2*s*d, sa*da - 2*(sa-s)*(da-d)) );
    }
#undef BLEND_MODE

// Returns ptr at (x,y).
template <typename T>
SI T* ptr_at_xy(const SkJumper_MemoryCtx* ctx, size_t x, size_t y) {
    return (T*)ctx->pixels + y*ctx->stride + x;
}

template <typename V, typename T>
SI V load(const T* ptr, size_t tail) {
    V v = 0;
    switch (tail & (N-1)) {
        case  0: memcpy(&v, ptr, sizeof(v)); break;
        case 15: v[14] = ptr[14];
        case 14: v[13] = ptr[13];
        case 13: v[12] = ptr[12];
        case 12: memcpy(&v, ptr, 12*sizeof(T)); break;
        case 11: v[10] = ptr[10];
        case 10: v[ 9] = ptr[ 9];
        case  9: v[ 8] = ptr[ 8];
        case  8: memcpy(&v, ptr,  8*sizeof(T)); break;
        case  7: v[ 6] = ptr[ 6];
        case  6: v[ 5] = ptr[ 5];
        case  5: v[ 4] = ptr[ 4];
        case  4: memcpy(&v, ptr,  4*sizeof(T)); break;
        case  3: v[ 2] = ptr[ 2];
        case  2: memcpy(&v, ptr,  2*sizeof(T)); break;
        case  1: v[ 0] = ptr[ 0];
    }
    return v;
}

SI void load_8888(const uint32_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
#if 1 && defined(__ARM_NEON)
    uint8x8x4_t rgba;
    switch (tail & (N-1)) {
        case 0: rgba = vld4_u8     ((const uint8_t*)(ptr+0)         ); break;
        case 7: rgba = vld4_lane_u8((const uint8_t*)(ptr+6), rgba, 6);
        case 6: rgba = vld4_lane_u8((const uint8_t*)(ptr+5), rgba, 5);
        case 5: rgba = vld4_lane_u8((const uint8_t*)(ptr+4), rgba, 4);
        case 4: rgba = vld4_lane_u8((const uint8_t*)(ptr+3), rgba, 3);
        case 3: rgba = vld4_lane_u8((const uint8_t*)(ptr+2), rgba, 2);
        case 2: rgba = vld4_lane_u8((const uint8_t*)(ptr+1), rgba, 1);
        case 1: rgba = vld4_lane_u8((const uint8_t*)(ptr+0), rgba, 0);
    }
    *r = vmovl_u8(rgba.val[0]);
    *g = vmovl_u8(rgba.val[1]);
    *b = vmovl_u8(rgba.val[2]);
    *a = vmovl_u8(rgba.val[3]);
#else
    U32 rgba = load<U32>(ptr, tail);
    *r = __builtin_convertvector((rgba >>  0) & 255, U16);
    *g = __builtin_convertvector((rgba >>  8) & 255, U16);
    *b = __builtin_convertvector((rgba >> 16) & 255, U16);
    *a = __builtin_convertvector((rgba >> 24) & 255, U16);
#endif
}

STAGE(load_8888, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, x,y), tail, &r,&g,&b,&a);
}
STAGE(load_8888_dst, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, x,y), tail, &dr,&dg,&db,&da);
}
STAGE(load_bgra, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, x,y), tail, &b,&g,&r,&a);
}
STAGE(load_bgra_dst, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, x,y), tail, &db,&dg,&dr,&da);
}

/*
SI void load_565(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
    __builtin_assume(tail < N);

}

STAGE(load_565_dst) {
    load_565(ptr_at_xy<const uint16_t>(ctx, x,y), tail, &dr,&dg,&db);
    da = 255;
}
*/
