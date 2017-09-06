/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"

#if defined(__clang__)
    template <int N, typename T>
    using V = T __attribute__((ext_vector_type(N)));

    #define SI static inline __attribute__((always_inline))
#else
    #define SI static inline
#endif

template <typename T, typename P>
SI T load(const P* p) {
    T v;
    memcpy(&v, p, sizeof(v));
    return v;
}
template <typename T, typename P>
SI void store(P* p, T v) {
    memcpy(p, &v, sizeof(v));
}

// *program++, in a single instruction on x86 when program is in %esi/%rsi.
SI void* load_and_inc(void**& program) {
#if defined(__x86_64__)
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));
    return rax;
#elif defined(__i386__)
    void* eax;
    asm("lodsd" : "=a"(eax), "+S"(program));
    return eax;
#else
    return *program++;
#endif
}

struct Ctx {
    using None = decltype(nullptr);

    explicit Ctx(void**& p) : program(p), ptr(nullptr) {}
    void**& program;
    void* ptr;

    template <typename T>
    operator T*() {
        if (!ptr) {
            ptr = load_and_inc(program);
        }
        return (T*)ptr;
    }

    operator None() { return nullptr; }
};

#if !defined(__clang__)
    #define HYBRID_IS_PORTABLE
#elif 0 && defined(__ARM_NEON)
    #define HYBRID_IS_NEON
#elif 0 && defined(__AVX2__)
    #define HYBRID_IS_AVX2
#elif 0 && defined(__AVX__)
    #define HYBRID_IS_AVX
#elif 0 && defined(__SSE4_1__)
    #define HYBRID_IS_SSE41
#elif 0 && defined(__SSE2__)
    #define HYBRID_IS_SSE2
#else
    #define HYBRID_IS_PORTABLE
#endif

// Each instruction set below may define stages to work with pixels at multiple precisions.
// We always require high-precision (float), but medium (e.g. half) or low (e.g. 8-bit)
// precision can slot in too.  When running a program, we'll work at the lowest precision
// that has all the program's stages defined.  (E.g. a fundamentally high-precision stage
// like to_srgb will kick the entire pipeline up to high precision.)
//
// You will see each mode below define H and maybe L, the number of pixels handled at
// high and low precision respectively.  (Nothing works at medium precision yet.)
// For each supported precision, we'll define an XY struct to carry pixel coordinates
// in at least float precision, and a Pixel struct to carry the pixels themselves.
//
// The coordinates or pixels flow from Stage to Stage, starting first with geometric
// manipulation on XY, and at some point switching over to working with src and dst Pixels.
// Stage is a single function that can carry either XY or two Pixels efficiently.
//
// H and L are mostly determined by the maximum number of vector registers we can pass
// through Stages while still fitting within one of Clang's available function ABIs and
// not spilling onto the stack.
//
// We assign rough types to the stages:
//   xG: requires no inputs, fills XY                   (basically only seed_shader)
//   xP: requires no inputs, fills src or dst Pixel     (e.g. a load from memory)
//   GG: transforms XY to XY'                           (e.g. a matrix)
//   PP: transforms src to src' or dst to dst'          (e.g. a color filter, a blend)
//   GP: transforms XY to src Pixel                     (e.g. a gather or a gradient)
//   Px: consumes src Pixel, terminating the program    (e.g. a store to memory)

#if defined(HYBRID_IS_PORTABLE)
    #include <stdint.h>

    // To keep things simple, we'll only define the minimum in this portable path, high precision.
    //static const int H = 1;
    using F = float;
    struct    XY_H { F x,y; };
    struct Pixel_H { float r,g,b,a; };

    // We'll use a uniform function type that's large enough to carry XY_H or two Pixel_H,
    using Stage = void(*)(int tail, void** program, int x, int y,
                          float f0, float f1, float f2, float f3,
                          float f4, float f5, float f6, float f7);

    #define STAGE_GG_H(name, ...)                                          \
        template <typename F, typename XY> SI void name##_T(__VA_ARGS__);  \
        extern "C" void name##_H(int tail, void** program, int x, int y,   \
                                 float f0, float f1, float f2, float f3,   \
                                 float f4, float f5, float f6, float f7) { \
            XY_H xy = { f0, f1 };                                          \
            Ctx ctx { program };                                           \
            name##_T<float>(ctx,x,y, &xy);                                 \
            auto next = (Stage)load_and_inc(program);                      \
            next(tail,program,x,y, xy.x,xy.y,f2,f3, f4,f5,f6,f7);          \
        }                                                                  \
        template <typename F, typename XY> SI void name##_T(__VA_ARGS__)
    #define STAGE_GG_L(name, ...) template <typename F, typename XY> SI void name##_L(__VA_ARGS__)
    #define STAGE_GG STAGE_GG_H

    #define STAGE_PP_H(name, ...)                                          \
        template <typename Pixel> SI void name##_T(__VA_ARGS__);           \
        extern "C" void name##_H(int tail, void** program, int x, int y,   \
                                 float f0, float f1, float f2, float f3,   \
                                 float f4, float f5, float f6, float f7) { \
            Pixel_H src = {f0,f1,f2,f3}, dst = {f4,f5,f6,f7};              \
            Ctx ctx { program };                                           \
            name##_T(ctx,x,y, &src,&dst);                                  \
            auto next = (Stage)load_and_inc(program);                      \
            next(tail,program,x,y, src.r,src.g,src.b,src.a,                \
                                   dst.r,dst.g,dst.b,dst.a);               \
        }                                                                  \
        template <typename Pixel> SI void name##_T(__VA_ARGS__)
    #define STAGE_PP_L(name, ...) template <typename Pixel> SI void name##_L(__VA_ARGS__)

#endif

STAGE_GG(seed_shader, Ctx::None,
         int x, int y, XY* xy) {
    float x_offsets[] = {
         0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5,
         8.5, 9.5,10.5,11.5,12.5,13.5,14.5,15.5,
        16.5,17.5,18.5,19.5,20.5,21.5,22.5,23.5,
        24.5,25.5,26.5,27.5,28.6,29.5,30.5,31.5,
    };
    xy->x = F(x) + load<F>(x_offsets);
    xy->y = F(y) + 0.5;
}

STAGE_PP_H(uniform_color, const SkJumper_UniformColorCtx* ctx,
           int x, int y, Pixel* src, Pixel* dst) {
    src->r = ctx->r;
    src->g = ctx->g;
    src->b = ctx->b;
    src->a = ctx->a;
}

STAGE_PP_L(uniform_color,
           const SkJumper_UniformColorCtx* ctx, int x, int y, Pixel* src, Pixel* dst) {
#if defined(HYBRID_IS_NEON)
    auto rgba = vld4_dup_u8((const uint8_t*)&ctx->rgba);
    src->r = rgba.r;
    src->g = rgba.g;
    src->b = rgba.b;
    src->a = rgba.a;
#else
    src->rgba = ctx->rgba;
#endif
}
