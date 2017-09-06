/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_hybrid_DEFINED
#define SkJumper_hybrid_DEFINED

// This file contains all the grungy setup that makes SkJumper_hybrid.cpp work.

#include "SkJumper.h"

#define SI static inline

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
template <typename D, typename S>
SI D pun(S src) {
    static_assert(sizeof(D) == sizeof(S), "");
    return load<D>(&src);
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
#elif 1 && defined(__ARM_NEON)
    #define HYBRID_IS_NEON
#elif 0 && defined(__AVX2__)
    #define HYBRID_IS_AVX2
#elif 0 && defined(__AVX__)
    #define HYBRID_IS_AVX
#elif 1 && defined(__SSE4_1__)
    #define HYBRID_IS_SSE41
#elif 1 && defined(__SSE2__)
    #define HYBRID_IS_SSE2
#else
    #define HYBRID_IS_PORTABLE
#endif

#if defined(__clang__)
    template <int N, typename T>
    using V = T __attribute__((ext_vector_type(N)));

    template <typename T>
    SI T uninitialized() {
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wuninitialized"
        T val;
        return val;
    #pragma clang diagnostic pop
    }

    template <int N, typename T>
    SI V<2*N,T> join(V<N,T> a, V<N,T> b) {
        V<2*N,T> joined;
        store((char*)&joined + 0*sizeof(V<N,T>), a);
        store((char*)&joined + 1*sizeof(V<N,T>), b);
        return joined;
    }
    template <int N, typename T>
    SI V<4*N,T> join(V<N,T> a, V<N,T> b, V<N,T> c, V<N,T> d) {
        V<4*N,T> joined;
        store((char*)&joined + 0*sizeof(V<N,T>), a);
        store((char*)&joined + 1*sizeof(V<N,T>), b);
        store((char*)&joined + 2*sizeof(V<N,T>), c);
        store((char*)&joined + 3*sizeof(V<N,T>), d);
        return joined;
    }

    template <int N, typename T>
    SI void split(V<2*N,T> joined, V<N,T>* a, V<N,T>* b) {
        memcpy(a, (const char*)&joined + 0*sizeof(V<N,T>), sizeof(V<N,T>));
        memcpy(b, (const char*)&joined + 1*sizeof(V<N,T>), sizeof(V<N,T>));
    }
    template <int N, typename T>
    SI void split(V<4*N,T> joined, V<N,T>* a, V<N,T>* b, V<N,T>* c, V<N,T>* d) {
        memcpy(a, (const char*)&joined + 0*sizeof(V<N,T>), sizeof(V<N,T>));
        memcpy(b, (const char*)&joined + 1*sizeof(V<N,T>), sizeof(V<N,T>));
        memcpy(c, (const char*)&joined + 2*sizeof(V<N,T>), sizeof(V<N,T>));
        memcpy(d, (const char*)&joined + 3*sizeof(V<N,T>), sizeof(V<N,T>));
    }
#endif

// Each instruction set below may define stages to work with pixels at multiple precisions.
// We always require high-precision (float), but medium (e.g. half) or low (e.g. 8-bit)
// precision can slot in too.  When running a program, we'll work at the lowest precision
// that has all the program's stages defined.  (E.g. a fundamentally high-precision stage
// like to_srgb will kick the entire pipeline up to high precision.)
//
// For each supported precision, we'll define an XY struct to carry pixel coordinates
// in at least float precision, and a Pixel struct to carry the pixels themselves.
//
// The coordinates or pixels flow from Stage to Stage, starting first with geometric
// manipulation on XY, and at some point switching over to working with src and dst Pixels.
// Stage is a single function that can carry either XY or two Pixels efficiently.
//
// We assign rough types to the stages:
//   GG: expects XY input (or ignores it), outputs XY'         (e.g. seed_shader, matrices)
//   GP: transforms XY to src Pixel                            (e.g. gathers, gradients)
//   PP: expects Pixel input (or ignores them), outputs Pixels (e.g. loads, blends, color filters)
//
// You really need to get all your geometric stages done first.  Especially at low-precision,
// we cannot simultaneously store XY and src and dst Pixels at once.
//
// H and L themselves are mostly determined by the maximum number of vector registers we
// can pass through Stages while still fitting XY or two Pixels within one of
// Clang's available function ABIs and not spilling onto the stack.

// We'll write pack(), unpack(), to make these work the same across all 8-register platforms.
#define STAGE_GG_8R(name, precision)                                           \
    extern "C" void name##_##precision(int tail, void** program, int x, int y, \
                                       R r0, R r1, R r2, R r3,                 \
                                       R r4, R r5, R r6, R r7) {               \
        precision::XY xy;                                                      \
        unpack(r0,r1,r2,r3,r4,r5,r6,r7, &xy);                                  \
        name##_T(Ctx{program},x,y, &xy);                                       \
        pack(xy, &r0,&r1,&r2,&r3,&r4,&r5,&r6,&r7);                             \
        auto next = (Stage)load_and_inc(program);                              \
        next(tail,program,x,y, r0,r1,r2,r3,r4,r5,r6,r7);                       \
    }                                                                          \

#define STAGE_PP_8R(name, precision)                                               \
        extern "C" void name##_##precision(int tail, void** program, int x, int y, \
                                           R r0, R r1, R r2, R r3,                 \
                                           R r4, R r5, R r6, R r7) {               \
            precision::Pixel src,dst;                                              \
            unpack(r0,r1,r2,r3, r4,r5,r6,r7, &src,&dst);                           \
            name##_##precision##T(Ctx{program},x,y, &src,&dst);                    \
            pack(src,dst, &r0,&r1,&r2,&r3, &r4,&r5,&r6,&r7);                       \
            auto next = (Stage)load_and_inc(program);                              \
            next(tail,program,x,y, r0,r1,r2,r3, r4,r5,r6,r7);                      \
        }                                                                          \

#if defined(HYBRID_IS_PORTABLE)
    #include <stdint.h>

    using R = float;
    using Stage = void(*)(int tail, void** program, int x, int y,
                          R r0, R r1, R r2, R r3,
                          R r4, R r5, R r6, R r7);

    // To keep things simple, in portable code we always work on one high-precision pixel.
    namespace H {
        struct XY { float x,y; };
        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, XY* xy) {
            *xy = {r0,r1};
        }
        SI void pack(XY xy, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            *r0 = xy.x;
            *r1 = xy.y;
        }

        struct Pixel { float r,g,b,a; };
        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, Pixel* src, Pixel* dst) {
            *src = {r0,r1,r2,r3};
            *dst = {r4,r5,r6,r7};
        }
        SI void pack(Pixel src, Pixel dst, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            *r0 = src.r; *r1 = src.g; *r2 = src.b; *r3 = src.a;
            *r4 = dst.r; *r5 = dst.g; *r6 = dst.b; *r7 = dst.a;
        }
    }

    #define STAGE_GG(name, ...)                               \
        template <typename XY> SI void name##_T(__VA_ARGS__); \
        STAGE_GG_8R(name,H)                                   \
        template <typename XY> SI void name##_T(__VA_ARGS__)

    #define STAGE_PP_H(name, ...)                                 \
        template <typename Pixel> SI void name##_HT(__VA_ARGS__); \
        STAGE_PP_8R(name,H)                                       \
        template <typename Pixel> SI void name##_HT(__VA_ARGS__)

    #define STAGE_PP_L(name, ...) \
        template <typename Pixel> SI void name##_LT(__VA_ARGS__)
        // I.e. just define a template but never use it, ignoring the code block.

#elif defined(HYBRID_IS_NEON)
    #include <arm_neon.h>

    using R = V<4,float>;
    using Stage = void(*)(int tail, void** program, int x, int y,
                          R r0, R r1, R r2, R r3,
                          R r4, R r5, R r6, R r7);

    namespace H {
        struct XY { V<4,float> x,y; };
        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, XY* xy) {
            *xy = {r0,r1};
        }
        SI void pack(XY xy, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            *r0 = xy.x;
            *r1 = xy.y;
        }

        struct Pixel { V<4,float> r,g,b,a; };
        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, Pixel* src, Pixel* dst) {
            *src = {r0,r1,r2,r3};
            *dst = {r4,r5,r6,r7};
        }
        SI void pack(Pixel src, Pixel dst, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            *r0 = src.r; *r1 = src.g; *r2 = src.b; *r3 = src.a;
            *r4 = dst.r; *r5 = dst.g; *r6 = dst.b; *r7 = dst.a;
        }
    }

    namespace L {
        struct XY { V<8,float> x,y; };
        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, XY* xy) {
            xy->x = join(r0,r1);
            xy->y = join(r2,r3);
        }
        SI void pack(XY xy, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            split(xy.x, r0, r1);
            split(xy.y, r2, r3);
        }

        using U8 = V<8,uint8_t>;
        struct Pixel { U8 r,g,b,a; };
        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, Pixel* src, Pixel* dst) {
            *src = { load<U8>(&r0), load<U8>(&r1), load<U8>(&r2), load<U8>(&r3) };
            *dst = { load<U8>(&r4), load<U8>(&r5), load<U8>(&r6), load<U8>(&r7) };
        }
        SI void pack(Pixel src, Pixel dst, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            auto cvt = [](float32x4_t* p, uint8x8_t v) {
                *p = vcombine_f32((float32x2_t)v, uninitialized<float32x2_t>());
            };
            cvt(r0, src.r); cvt(r1, src.g); cvt(r2, src.b); cvt(r3, src.a);
            cvt(r4, dst.r); cvt(r5, dst.g); cvt(r6, dst.b); cvt(r7, dst.a);
        }
    }

    #define STAGE_GG(name, ...)                               \
        template <typename XY> SI void name##_T(__VA_ARGS__); \
        STAGE_GG_8R(name,H)                                   \
        STAGE_GG_8R(name,L)                                   \
        template <typename XY> SI void name##_T(__VA_ARGS__)

    #define STAGE_PP_H(name, ...)                                        \
        template <typename Pixel> SI void name##_HT(__VA_ARGS__);        \
        STAGE_PP_8R(name, H)                                             \
        template <typename Pixel> SI void name##_HT(__VA_ARGS__)

    #define STAGE_PP_L(name, ...)                                        \
        template <typename Pixel> SI void name##_LT(__VA_ARGS__);        \
        STAGE_PP_8R(name, L)                                             \
        template <typename Pixel> SI void name##_LT(__VA_ARGS__)

#elif defined(HYBRID_IS_SSE2) || defined(HYBRID_IS_SSE41)
    #include <immintrin.h>

    using R = V<4,float>;
    using Stage = void(*)(int tail, void** program, int x, int y,
                          R r0, R r1, R r2, R r3,
                          R r4, R r5, R r6, R r7);
    namespace H {
        struct XY    { V<4,float> x,y; };
        struct Pixel { V<4,float> r,g,b,a; };

        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, XY* xy) {
            xy->x = r0;
            xy->y = r1;
        }
        SI void pack(XY xy, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            *r0 = xy.x;
            *r1 = xy.y;
        }
    }

    namespace L {
        struct XY    { V<16,float> x,y; };
        struct Pixel { V<16,uint32_t> rgba; };

        SI void unpack(R r0, R r1, R r2, R r3, R r4, R r5, R r6, R r7, XY* xy) {
            xy->x = join(r0,r1,r2,r3);
            xy->y = join(r4,r5,r6,r7);
        }
        SI void pack(XY xy, R* r0, R* r1, R* r2, R* r3, R* r4, R* r5, R* r6, R* r7) {
            split(xy.x, r0,r1,r2,r3);
            split(xy.y, r4,r5,r6,r7);
        }
    }

    #define STAGE_GG(name, ...)                               \
        template <typename XY> SI void name##_T(__VA_ARGS__); \
        STAGE_GG_8R(name,H)                                   \
        STAGE_GG_8R(name,L)                                   \
        template <typename XY> SI void name##_T(__VA_ARGS__)

    #define STAGE_PP_H(name, ...)                                        \
        template <typename Pixel> SI void name##_HT(__VA_ARGS__);        \
        extern "C" void name##_H(int tail, void** program, int x, int y, \
                                 R r0, R r1, R r2, R r3,                 \
                                 R r4, R r5, R r6, R r7) {               \
            H::Pixel src = {r0,r1,r2,r3}, dst = {r4,r5,r6,r7};           \
            name##_HT(Ctx{program},x,y, &src,&dst);                      \
            auto next = (Stage)load_and_inc(program);                    \
            next(tail,program,x,y, src.r,src.g,src.b,src.a,              \
                                   dst.r,dst.g,dst.b,dst.a);             \
        }                                                                \
        template <typename Pixel> SI void name##_HT(__VA_ARGS__)

    #define STAGE_PP_L(name, ...)                                        \
        template <typename Pixel> SI void name##_LT(__VA_ARGS__);        \
        extern "C" void name##_L(int tail, void** program, int x, int y, \
                                 R r0, R r1, R r2, R r3,                 \
                                 R r4, R r5, R r6, R r7) {               \
            L::Pixel src = { pun<V<16,uint32_t>>(join(r0,r1,r2,r3)) },   \
                     dst = { pun<V<16,uint32_t>>(join(r4,r5,r6,r7)) };   \
            name##_LT(Ctx{program},x,y, &src,&dst);                      \
            split(pun<V<16,float>>(src), &r0,&r1,&r2,&r3);               \
            split(pun<V<16,float>>(dst), &r4,&r5,&r6,&r7);               \
            auto next = (Stage)load_and_inc(program);                    \
            next(tail,program,x,y, r0,r1,r2,r3, r4,r5,r6,r7);            \
        }                                                                \
        template <typename Pixel> SI void name##_LT(__VA_ARGS__)

#endif


#endif//SkJumper_hybrid_DEFINED
