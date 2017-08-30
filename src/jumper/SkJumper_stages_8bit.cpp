/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This restricted SkJumper backend works on 8-bit per channel pixels.
// This is the natural format for kN32_SkColorType buffers, and we
// hope the stages in this file can replace many custom legacy routines.

#include "SkJumper.h"
#include "SkJumper_misc.h"

// Used by load_ and store_ stages to get to the right (x,y) starting point of contiguous memory.
template <typename T>
SI T* ptr_at_xy(const SkJumper_MemoryCtx* ctx, int x, int y) {
    return (T*)ctx->pixels + y*ctx->stride + x;
}

#if defined(JUMPER_IS_OFFLINE)  // We compile x86 8-bit stages offline.
    #include <immintrin.h>
    #if defined(__AVX2__)
        #define WRAP(name) sk_##name##_hsw_8bit
    #elif defined(__SSE4_1__)
        #define WRAP(name) sk_##name##_sse41_8bit
    #elif defined(__SSE2__)
        #define WRAP(name) sk_##name##_sse2_8bit
    #endif

    // We're going to work with pixels transposed just as they'd be in memory,
    // generally either considering them U8x4 (r,g,b,a) or as U32 (rgba) values.

    #if defined(__AVX2__)
        using U8    = uint8_t  __attribute__((ext_vector_type(16)));
        using U32   = uint32_t __attribute__((ext_vector_type(16)));
        using U8x4  = uint8_t  __attribute__((ext_vector_type(64)));
        using U16x4 = uint16_t __attribute__((ext_vector_type(64)));
        using R     = uint8_t  __attribute__((ext_vector_type(32)));
    #else
        using U8    = uint8_t  __attribute__((ext_vector_type( 8)));
        using U32   = uint32_t __attribute__((ext_vector_type( 8)));
        using U8x4  = uint8_t  __attribute__((ext_vector_type(32)));
        using U16x4 = uint16_t __attribute__((ext_vector_type(32)));
        using R     = uint8_t  __attribute__((ext_vector_type(16)));
    #endif

    // We double pump our math, making each U32 or U8x4 twice as wide as a native
    // vector register, and each U16x4 occupy four.
    //
    // These would be tricky to pass around directly because of ABI restrictions,
    // so we split them across two R to pass data between stages.  This is
    // typically only a virtual operation, with no runtime cost.
    SI U8x4 join(R lo, R hi) {
        U8x4 u8x4;
        memcpy((char*)&u8x4            , &lo, sizeof(R));
        memcpy((char*)&u8x4 + sizeof(R), &hi, sizeof(R));
        return u8x4;
    }
    SI void split(U8x4 u8x4, R* lo, R* hi) {
        memcpy(lo, (char*)&u8x4            , sizeof(R));
        memcpy(hi, (char*)&u8x4 + sizeof(R), sizeof(R));
    }

    // Usually __builtin_convertvector() is pretty good, but sometimes we can do better.
    SI U8x4 pack(U16x4 v) {
    #if defined(__AVX2__)
        static_assert(sizeof(v) == 128, "");
        auto A = unaligned_load<__m256i>((char*)&v +  0),
             B = unaligned_load<__m256i>((char*)&v + 32),
             C = unaligned_load<__m256i>((char*)&v + 64),
             D = unaligned_load<__m256i>((char*)&v + 96);

        auto pack = [](__m256i lo, __m256i hi) {
            auto _02 = _mm256_permute2x128_si256(lo,hi, 0x20),
                 _13 = _mm256_permute2x128_si256(lo,hi, 0x31);
            return _mm256_packus_epi16(_02, _13);
        };
        return join(pack(A,B), pack(C,D));
    #elif defined(__SSE2__)
        static_assert(sizeof(v) == 64, "");
        auto A = unaligned_load<__m128i>((char*)&v +  0),
             B = unaligned_load<__m128i>((char*)&v + 16),
             C = unaligned_load<__m128i>((char*)&v + 32),
             D = unaligned_load<__m128i>((char*)&v + 48);
        return join(_mm_packus_epi16(A,B), _mm_packus_epi16(C,D));
    #else
        return __builtin_convertvector(v, U8x4);
    #endif
    }

    union V {
        U32  u32;
        U8x4 u8x4;

        V() = default;
        V(U32   v) : u32 (v) {}
        V(U8x4  v) : u8x4(v) {}
        V(U16x4 v) : u8x4(pack((v + 127)/255)) {}
        V(int   v) : u8x4(v) {}
        V(float v) : u8x4(v*255) {}
    };
    static const size_t kStride = sizeof(V) / sizeof(uint32_t);

    SI V operator+(V x, V y) { return x.u8x4 + y.u8x4; }
    SI V operator-(V x, V y) { return x.u8x4 - y.u8x4; }
    SI V operator*(V x, V y) {
        // (x*y + x)/256 is a very good approximation of (x*y + 127)/255.
        U16x4 X = __builtin_convertvector(x.u8x4, U16x4),
              Y = __builtin_convertvector(y.u8x4, U16x4);
        return pack((X*Y + X)>>8);
    }

    template <typename T>
    SI T inv(T v) { return 0xff - v; }

    SI V lerp(V from, V to, V t) { return to*t + from*inv(t); }

    SI V alpha(V v) {
    #if defined(__AVX2__)
        return __builtin_shufflevector(v.u8x4,v.u8x4,
                                        3, 3, 3, 3,  7, 7, 7, 7, 11,11,11,11, 15,15,15,15,
                                       19,19,19,19, 23,23,23,23, 27,27,27,27, 31,31,31,31,
                                       35,35,35,35, 39,39,39,39, 43,43,43,43, 47,47,47,47,
                                       51,51,51,51, 55,55,55,55, 59,59,59,59, 63,63,63,63);
    #else
        return __builtin_shufflevector(v.u8x4,v.u8x4,
                                        3, 3, 3, 3,  7, 7, 7, 7, 11,11,11,11, 15,15,15,15,
                                       19,19,19,19, 23,23,23,23, 27,27,27,27, 31,31,31,31);
    #endif
    }

    SI V swap_rb(V v) {
    #if defined(__AVX2__)
        return __builtin_shufflevector(v.u8x4,v.u8x4,
                                        2, 1, 0, 3,  6, 5, 4, 7, 10, 9, 8,11, 14,13,12,15,
                                       18,17,16,19, 22,21,20,23, 26,25,24,27, 30,29,28,31,
                                       34,33,32,35, 38,37,36,39, 42,41,40,43, 46,45,44,47,
                                       50,49,48,51, 54,53,52,55, 58,57,56,59, 62,61,60,63);
    #else
        return __builtin_shufflevector(v.u8x4,v.u8x4,
                                        2, 1, 0, 3,  6, 5, 4, 7, 10, 9, 8,11, 14,13,12,15,
                                       18,17,16,19, 22,21,20,23, 26,25,24,27, 30,29,28,31);
    #endif
    }


    template <typename MaskT, typename ValT>
    SI ValT if_then_else(MaskT m, ValT t, ValT e) {
        return (t & m) | (e & ~m);
    }
    SI V max(V a, V b) { return if_then_else(a.u8x4 > b.u8x4, a.u8x4, b.u8x4); }
    SI V min(V a, V b) { return if_then_else(a.u8x4 > b.u8x4, b.u8x4, a.u8x4); }

    SI V saturated_add(V a, V b) {
        R a_lo, a_hi,
          b_lo, b_hi;
        split(a.u8x4, &a_lo, &a_hi);
        split(b.u8x4, &b_lo, &b_hi);
    #if defined(__AVX2__)
        return join(_mm256_adds_epu8(a_lo, b_lo),
                    _mm256_adds_epu8(a_hi, b_hi));
    #elif defined(__SSE2__)
        return join(_mm_adds_epu8(a_lo, b_lo),
                    _mm_adds_epu8(a_hi, b_hi));
    #endif
    }

    // We pass program as the second argument to keep it in rsi for load_and_inc().
    using Stage = void(*)(size_t tail, void** program, size_t x, size_t y,
                          R src_lo, R src_hi, R dst_lo, R dst_hi);

    #if defined(__AVX__)
        // We really want to make sure all paths go through this function's (implicit) vzeroupper.
        // If they don't, we'll experience severe slowdowns when we first use SSE again.
        __attribute__((disable_tail_calls))
    #endif
    MAYBE_MSABI
    extern "C" void WRAP(start_pipeline)(size_t x, size_t y, size_t xlimit, size_t ylimit,
                                         void** program) {
    #if defined(JUMPER_IS_OFFLINE)
        R r;   // Fastest to start uninitialized.
    #else
        R r{}; // Next best is zero'd for compilers that will complain about uninitialized values.
    #endif
        auto start = (Stage)load_and_inc(program);
        const size_t x0 = x;
        for (; y < ylimit; y++) {
            x = x0;
            while (x + kStride <= xlimit) {
                start(0,program,x,y, r,r,r,r);
                x += kStride;
            }
            if (size_t tail = xlimit - x) {
                start(tail,program,x,y, r,r,r,r);
            }
        }
    }

    extern "C" void WRAP(just_return)(size_t,void**,size_t,size_t, R,R,R,R) {}

    #define STAGE(name)                                                                  \
        SI void name##_k(LazyCtx ctx, size_t x, size_t y, size_t tail, V& src, V& dst);  \
        extern "C" void WRAP(name)(size_t tail, void** program, size_t x, size_t y,      \
                                   R src_lo, R src_hi, R dst_lo, R dst_hi) {             \
            V src = join(src_lo, src_hi),                                                \
              dst = join(dst_lo, dst_hi);                                                \
            LazyCtx ctx(program);                                                        \
            name##_k(ctx, x,y,tail, src, dst);                                           \
            split(src.u8x4, &src_lo, &src_hi);                                           \
            split(dst.u8x4, &dst_lo, &dst_hi);                                           \
            auto next = (Stage)load_and_inc(program);                                    \
            next(tail,program,x,y, src_lo,src_hi, dst_lo,dst_hi);                        \
        }                                                                                \
        SI void name##_k(LazyCtx ctx, size_t x, size_t y, size_t tail, V& src, V& dst)

    template <typename V, typename T>
    SI V load(const T* src, size_t tail) {
        __builtin_assume(tail < kStride);
        if (__builtin_expect(tail, 0)) {
            V v = 0;
            switch (tail) {
            #if defined(__AVX2__)
                case 15: v[14] = src[14];
                case 14: v[13] = src[13];
                case 13: v[12] = src[12];
                case 12: memcpy(&v, src, 12*sizeof(T)); break;
                case 11: v[10] = src[10];
                case 10: v[ 9] = src[ 9];
                case  9: v[ 8] = src[ 8];
                case  8: memcpy(&v, src, 8*sizeof(T)); break;
            #endif
                case  7: v[6] = src[6];
                case  6: v[5] = src[5];
                case  5: v[4] = src[4];
                case  4: memcpy(&v, src, 4*sizeof(T)); break;
                case  3: v[2] = src[2];
                case  2: memcpy(&v, src, 2*sizeof(T)); break;
                case  1: memcpy(&v, src, 1*sizeof(T)); break;
            }
            return v;
        }
        return unaligned_load<V>(src);
    }

    template <typename V, typename T>
    SI void store(T* dst, V v, size_t tail) {
        __builtin_assume(tail < kStride);
        if (__builtin_expect(tail, 0)) {
            switch (tail) {
            #if defined(__AVX2__)
                case 15: dst[14] = v[14];
                case 14: dst[13] = v[13];
                case 13: dst[12] = v[12];
                case 12: memcpy(dst, &v, 12*sizeof(T)); break;
                case 11: dst[10] = v[10];
                case 10: dst[ 9] = v[ 9];
                case  9: dst[ 8] = v[ 8];
                case  8: memcpy(dst, &v, 8*sizeof(T)); break;
            #endif
                case  7: dst[6] = v[6];
                case  6: dst[5] = v[5];
                case  5: dst[4] = v[4];
                case  4: memcpy(dst, &v, 4*sizeof(T)); break;
                case  3: dst[2] = v[2];
                case  2: memcpy(dst, &v, 2*sizeof(T)); break;
                case  1: memcpy(dst, &v, 1*sizeof(T)); break;
            }
            return;
        }
        unaligned_store(dst, v);
    }


    STAGE(uniform_color) {
        auto c = (const SkJumper_UniformColorCtx*)ctx;
        src.u32 = c->rgba;
    }

    STAGE(set_rgb) {
        auto c = (const float*)ctx;

        src.u32 = (uint32_t)(c[0] * 255) << 0
                | (uint32_t)(c[1] * 255) << 8
                | (uint32_t)(c[2] * 255) << 16
                | (src.u32 & 0xff000000);
    }

    STAGE(premul) {
        // I.e. rgb *= a, a *= 1.0f.
        src = src * (alpha(src).u32 | 0xff000000);
    }
    STAGE(swap_rb) {
        src = swap_rb(src);
    }
    STAGE(invert) {
        src = inv(src);
    }

    STAGE(load_8888) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
        src = load<U32>(ptr, tail);
    }
    STAGE(load_8888_dst) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
        dst = load<U32>(ptr, tail);
    }
    STAGE(store_8888) {
        auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);
        store(ptr, src.u32, tail);
    }

    STAGE(load_bgra) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
        src = swap_rb(load<U32>(ptr, tail));
    }
    STAGE(load_bgra_dst) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
        dst = swap_rb(load<U32>(ptr, tail));
    }
    STAGE(store_bgra) {
        auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);
        store(ptr, swap_rb(src).u32, tail);
    }

    STAGE(load_a8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
        src = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
    }
    STAGE(load_a8_dst) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
        dst = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
    }
    STAGE(store_a8) {
        auto ptr = ptr_at_xy<uint8_t>(ctx, x,y);
        store(ptr, __builtin_convertvector(src.u32 >> 24, U8), tail);
    }

    STAGE(load_g8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
        src = (__builtin_convertvector(load<U8>(ptr, tail), U32) * 0x010101) | 0xff000000;
    }
    STAGE(load_g8_dst) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
        dst = (__builtin_convertvector(load<U8>(ptr, tail), U32) * 0x010101) | 0xff000000;
    }

    STAGE(srcover_rgba_8888) {
        auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

        V d = load<U32>(ptr, tail);
        V b = src + (d - d*alpha(src));

        store(ptr, b.u32, tail);
    }

    STAGE(scale_1_float) {
        float c = *(const float*)ctx;
        src = src * c;
    }
    STAGE(scale_u8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

        V c = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
        src = src * alpha(c);
    }

    STAGE(lerp_1_float) {
        float c = *(const float*)ctx;
        src = lerp(dst, src, c);
    }
    STAGE(lerp_u8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

        V c = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
        src = lerp(dst, src, alpha(c));
    }

    STAGE(move_src_dst) { dst = src; }
    STAGE(move_dst_src) { src = dst; }

    STAGE(black_color) { src.u32 = 0xff000000; }
    STAGE(white_color) { src.u32 = 0xffffffff; }
    STAGE(clear)       { src.u32 = 0x00000000; }

    STAGE(srcatop)  { src = src*alpha(dst) + dst*inv(alpha(src)); }
    STAGE(dstatop)  { src = dst*alpha(src) + src*inv(alpha(dst)); }
    STAGE(srcin)    { src = src * alpha(dst); }
    STAGE(dstin)    { src = dst * alpha(src); }
    STAGE(srcout)   { src = src * inv(alpha(dst)); }
    STAGE(dstout)   { src = dst * inv(alpha(src)); }
    STAGE(srcover)  { src = src + (dst - dst*alpha(src)); }
    STAGE(dstover)  { src = dst + (src - src*alpha(dst)); }
    STAGE(modulate) { src = src*dst; }
    STAGE(multiply) { src = src*inv(alpha(dst)) + dst*inv(alpha(src)) + src*dst; }
    STAGE(screen)   { src = src + inv(src)*dst; }
    STAGE(xor_)     { src = src*inv(alpha(dst)) + dst*inv(alpha(src)); }
    STAGE(plus_)    { src = saturated_add(src, dst); }

    SI V srcover_alpha(V src, V dst, V rgb) {
        V a = src + (dst - dst*alpha(src));
        return (rgb.u32 & 0x00ffffff) | (a.u32 & 0xff000000);
    }

    STAGE(darken) {
        src = srcover_alpha(src, dst, src + (dst - max(src*alpha(dst), dst*alpha(src))));
    }
    STAGE(lighten) {
        src = srcover_alpha(src, dst, src + (dst - min(src*alpha(dst), dst*alpha(src))));
    }

    SI V zero_alpha(V rgba) { return rgba.u32 & 0x00ffffff; }

    STAGE(exclusion) {
        // We could do exclusion with srcover_alpha(), but can fold a little more math through:
        //     rgb   = Sc + Dc - 2*Sc*Dc
        //     alpha = Sa + Da -   Sa*Da
        // So we just subtract two sd from rgb, and one from alpha.
        V sd = src*dst;
        src = (src - sd) + (dst - zero_alpha(sd));
    }
    STAGE(difference) {
        // Like exclusion, we can fold math through with the same trick:
        //     rgb   = Sc + Dc - 2*min(Sc*Da, Dc*Sa)
        //     alpha = Sa + Da -       Sa*Da
        // Here notice (Sa*Da) == min(Sa*Da, Da*Sa) for alpha,
        // so again we subtract two from rgb, one from alpha.
        V min_ = min(src*alpha(dst), dst*alpha(src));
        src = (src - min_) + (dst - zero_alpha(min_));
    }

    template <typename Func>
    V blend_rgb16(V src, V dst, Func&& blend) {
        U16x4 s   = __builtin_convertvector(       src.u8x4, U16x4),
              sa  = __builtin_convertvector(alpha(src).u8x4, U16x4),
              d   = __builtin_convertvector(       dst.u8x4, U16x4),
              da  = __builtin_convertvector(alpha(dst).u8x4, U16x4),

              rgb = blend(s, d, sa, da),
              a   = s + (d - d*sa);

        return if_then_else(0x0000ffffffffffff, rgb, a);
    }

    STAGE(hardlight) {
        src = blend_rgb16(src, dst, [](U16x4 s, U16x4 d, U16x4 sa, U16x4 da) {
            return s*inv(da) + d*inv(sa)
                 + if_then_else(s*2 <= sa, s*d*2, sa*da - (da - d)*(sa - s)*2);
        });
    }

    STAGE(overlay) {
        src = blend_rgb16(src, dst, [](U16x4 s, U16x4 d, U16x4 sa, U16x4 da) {
            return s*inv(da) + d*inv(sa)
                 + if_then_else(d*2 <= da, s*d*2, sa*da - (da - d)*(sa - s)*2);
        });
    }

#elif defined(JUMPER_HAS_NEON_8BIT)  // These are generally compiled as part of Skia.
    #include <arm_neon.h>

    #if defined(__arm__)
        #define ABI __attribute__((pcs("aapcs-vfp")))
    #else
        #define ABI
    #endif

    #define WRAP(name) sk_##name##_8bit

    // On ARM it's so easy to de-interlace on loads and re-interlace on stores that
    // we'll be working with uniform 8x r,g,b,a vectors, much like the float code.

    using U8  = uint8_t  __attribute__((ext_vector_type(8)));
    using U16 = uint16_t __attribute__((ext_vector_type(8)));
    static const size_t kStride = sizeof(U8) / sizeof(uint8_t);

    // V is basically U8, but we make operator* a unorm8 multiply (x*y+127)/255.
    struct V {
        U8 vec;

        V() = default;
        V(U8    v) : vec(v) {}
        V(int   v) : vec(v) {}
        V(float v) : vec(v * 255) {}
        V(U16   v) {
        #if 0
            // (v + 127) / 255 = (v + ((v+128)>>8) + 128) >> 8
            vec = vraddhn_u16(v, vrshrq_n_u16(v, 8));
        #else
            // (v + 127) / 255 ≈ (v + 255) >> 8
            vec = vaddhn_u16(v, U16(255));
        #endif
        }

        operator U8() const { return vec; }
    };

    SI U16 mul_wide(V x, V y) { return vmull_u8(x,y); }

    SI V operator+(V x, V y) { return x.vec + y.vec; }
    SI V operator-(V x, V y) { return x.vec - y.vec; }
    SI V operator*(V x, V y) { return mul_wide(x,y); }

    SI V inv(V x) { return 1.0f - x; }
    SI V lerp(V from, V to, V t) { return to*t + from*inv(t); }

    template <typename MaskT, typename ValT>
    SI ValT if_then_else(MaskT m, ValT t, ValT e) {
        return (t & m) | (e & ~m);
    }
    SI V max(V a, V b) { return if_then_else(a > b, a.vec, b.vec); }
    SI V min(V a, V b) { return if_then_else(a > b, b.vec, a.vec); }


    // We need to pass as U8 (raw vector types unwrapped by any struct) to appease ARMv7's ABI.
    using Stage = void (ABI *)(void** program, size_t x, size_t y, size_t tail,
                               U8  r, U8  g, U8  b, U8  a,
                               U8 dr, U8 dg, U8 db, U8 da);

    ABI extern "C" void WRAP(start_pipeline)(size_t x, size_t y, size_t xlimit, size_t ylimit,
                                             void** program) {
        V v{};
        auto start = (Stage)load_and_inc(program);
        const size_t x0 = x;
        for (; y < ylimit; y++) {
            x = x0;
            while (x + kStride <= xlimit) {
                start(program,x,y,0,    v,v,v,v, v,v,v,v);
                x += kStride;
            }
            if (size_t tail = xlimit - x) {
                start(program,x,y,tail, v,v,v,v, v,v,v,v);
            }
        }
    }

    ABI extern "C" void WRAP(just_return)(void**,size_t,size_t,size_t,
                                          U8,U8,U8,U8, U8,U8,U8,U8) {}

    #define STAGE(name)                                                                  \
        SI void name##_k(LazyCtx ctx, size_t x, size_t y, size_t tail,                   \
                         V&  r, V&  g, V&  b, V&  a,                                     \
                         V& dr, V& dg, V& db, V& da);                                    \
        ABI extern "C" void WRAP(name)(void** program, size_t x, size_t y, size_t tail,  \
                                       U8  r, U8  g, U8  b, U8  a,                       \
                                       U8 dr, U8 dg, U8 db, U8 da) {                     \
            LazyCtx ctx(program);                                                        \
            V R =  r,  G =  g,  B =  b,  A =  a,                                         \
             DR = dr, DG = dg, DB = db, DA = da;                                         \
            name##_k(ctx,x,y,tail, R,G,B,A, DR,DG,DB,DA);                                \
            auto next = (Stage)load_and_inc(program);                                    \
            next(program, x,y,tail, R,G,B,A, DR,DG,DB,DA);                               \
        }                                                                                \
        SI void name##_k(LazyCtx ctx, size_t x, size_t y, size_t tail,                   \
                         V&  r, V&  g, V&  b, V&  a,                                     \
                         V& dr, V& dg, V& db, V& da)

    STAGE(uniform_color) {
        auto c = (const SkJumper_UniformColorCtx*)ctx;

        auto rgba = vld4_dup_u8((const uint8_t*)&c->rgba);
        r = rgba.val[0];
        g = rgba.val[1];
        b = rgba.val[2];
        a = rgba.val[3];
    }

    STAGE(set_rgb) {
        auto c = (const float*)ctx;

        r = c[0];
        g = c[1];
        b = c[2];
    }

    STAGE(premul) {
        r = r * a;
        g = g * a;
        b = b * a;
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

    SI uint8x8x4_t load_u32(const uint32_t* ptr, size_t tail) {
        __builtin_assume(tail < kStride);
        uint8x8x4_t rgba;
        switch (tail) {
            case 0: rgba = vld4_u8((const uint8_t*)ptr); break;
            case 7: rgba = vld4_lane_u8((const uint8_t*)(ptr+6), rgba, 6);
            case 6: rgba = vld4_lane_u8((const uint8_t*)(ptr+5), rgba, 5);
            case 5: rgba = vld4_lane_u8((const uint8_t*)(ptr+4), rgba, 4);
            case 4: rgba = vld4_lane_u8((const uint8_t*)(ptr+3), rgba, 3);
            case 3: rgba = vld4_lane_u8((const uint8_t*)(ptr+2), rgba, 2);
            case 2: rgba = vld4_lane_u8((const uint8_t*)(ptr+1), rgba, 1);
            case 1: rgba = vld4_lane_u8((const uint8_t*)(ptr+0), rgba, 0);
        }
        return rgba;
    }
    SI void store_u32(uint32_t* ptr, size_t tail, uint8x8x4_t rgba) {
        __builtin_assume(tail < kStride);
        switch (tail) {
            case 0: vst4_u8((uint8_t*)ptr, rgba); break;
            case 7: vst4_lane_u8((uint8_t*)(ptr+6), rgba, 6);
            case 6: vst4_lane_u8((uint8_t*)(ptr+5), rgba, 5);
            case 5: vst4_lane_u8((uint8_t*)(ptr+4), rgba, 4);
            case 4: vst4_lane_u8((uint8_t*)(ptr+3), rgba, 3);
            case 3: vst4_lane_u8((uint8_t*)(ptr+2), rgba, 2);
            case 2: vst4_lane_u8((uint8_t*)(ptr+1), rgba, 1);
            case 1: vst4_lane_u8((uint8_t*)(ptr+0), rgba, 0);
        }
    }

    SI U8 load_u8(const uint8_t* ptr, size_t tail) {
        __builtin_assume(tail < kStride);
        U8 v = 0;
        switch (tail) {
            case 0: memcpy(&v, ptr, 8); break;
            case 7: v[6] = ptr[6];
            case 6: v[5] = ptr[5];
            case 5: v[4] = ptr[4];
            case 4: memcpy(&v, ptr, 4); break;
            case 3: v[2] = ptr[2];
            case 2: memcpy(&v, ptr, 2); break;
            case 1: v[0] = ptr[0];
        }
        return v;
    }
    SI void store_u8(uint8_t* ptr, size_t tail, U8 v) {
        __builtin_assume(tail < kStride);
        switch (tail) {
            case 0: memcpy(ptr, &v, 8); break;
            case 7: ptr[6] = v[6];
            case 6: ptr[5] = v[5];
            case 5: ptr[4] = v[4];
            case 4: memcpy(ptr, &v, 4); break;
            case 3: ptr[2] = v[2];
            case 2: memcpy(ptr, &v, 2); break;
            case 1: ptr[0] = v[0];
        }
    }

    STAGE(load_8888) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);

        auto rgba = load_u32(ptr, tail);
        r = rgba.val[0];
        g = rgba.val[1];
        b = rgba.val[2];
        a = rgba.val[3];
    }
    STAGE(load_8888_dst) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);

        auto rgba = load_u32(ptr, tail);
        dr = rgba.val[0];
        dg = rgba.val[1];
        db = rgba.val[2];
        da = rgba.val[3];
    }
    STAGE(store_8888) {
        auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

        uint8x8x4_t rgba = {{ r,g,b,a }};
        store_u32(ptr, tail, rgba);
    }

    STAGE(load_bgra) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);

        auto rgba = load_u32(ptr, tail);
        b = rgba.val[0];
        g = rgba.val[1];
        r = rgba.val[2];
        a = rgba.val[3];
    }
    STAGE(load_bgra_dst) {
        auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);

        auto rgba = load_u32(ptr, tail);
        db = rgba.val[0];
        dg = rgba.val[1];
        dr = rgba.val[2];
        da = rgba.val[3];
    }
    STAGE(store_bgra) {
        auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

        uint8x8x4_t rgba = {{ b,g,r,a }};
        store_u32(ptr, tail, rgba);
    }

    STAGE(load_a8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

        r = g = b = 0;
        a = load_u8(ptr, tail);
    }
    STAGE(load_a8_dst) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

        dr = dg = db = 0;
        da = load_u8(ptr, tail);
    }
    STAGE(store_a8) {
        auto ptr = ptr_at_xy<uint8_t>(ctx, x,y);
        store_u8(ptr, tail, a);
    }

    STAGE(load_g8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
        r = g = b = load_u8(ptr, tail);
        a = 1.0f;
    }
    STAGE(load_g8_dst) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
        dr = dg = db = load_u8(ptr, tail);
        da = 1.0f;
    }

    STAGE(srcover_rgba_8888) {
        auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

        auto dst = load_u32(ptr, tail);
        r = r + (V)dst.val[0]*inv(a);
        g = g + (V)dst.val[1]*inv(a);
        b = b + (V)dst.val[2]*inv(a);
        a = a + (V)dst.val[3]*inv(a);

        uint8x8x4_t rgba = {{r,g,b,a}};
        store_u32(ptr, tail, rgba);
    }

    STAGE(scale_1_float) {
        float c = *(const float*)ctx;
        r = r * c;
        g = g * c;
        b = b * c;
        a = a * c;
    }
    STAGE(scale_u8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

        V c = load_u8(ptr, tail);
        r = r * c;
        g = g * c;
        b = b * c;
        a = a * c;
    }

    STAGE(lerp_1_float) {
        float c = *(const float*)ctx;

        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
    }
    STAGE(lerp_u8) {
        auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

        V c = load_u8(ptr, tail);
        r = lerp(dr, r, c);
        g = lerp(dg, g, c);
        b = lerp(db, b, c);
        a = lerp(da, a, c);
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

    STAGE(black_color) { r = g = b = 0.0f; a = 1.0f; }
    STAGE(white_color) { r = g = b = 1.0f; a = 1.0f; }

    // Most blend modes apply the same logic to each channel.
    #define BLEND_MODE(name)                       \
        SI V name##_channel(V s, V d, V sa, V da); \
        STAGE(name) {                              \
            r = name##_channel(r,dr,a,da);         \
            g = name##_channel(g,dg,a,da);         \
            b = name##_channel(b,db,a,da);         \
            a = name##_channel(a,da,a,da);         \
        }                                          \
        SI V name##_channel(V s, V d, V sa, V da)

    BLEND_MODE(clear)    { return 0; }
    BLEND_MODE(srcatop)  { return s*da + d*inv(sa); }
    BLEND_MODE(dstatop)  { return d*sa + s*inv(da); }
    BLEND_MODE(srcin)    { return s*da; }
    BLEND_MODE(dstin)    { return d*sa; }
    BLEND_MODE(srcout)   { return s * inv(da); }
    BLEND_MODE(dstout)   { return d * inv(sa); }
    BLEND_MODE(srcover)  { return s + d*inv(sa); }
    BLEND_MODE(dstover)  { return d + s*inv(da); }
    BLEND_MODE(modulate) { return s*d; }
    BLEND_MODE(multiply) { return s*inv(da) + d*inv(sa) + s*d; }
    BLEND_MODE(plus_)    { return vqadd_u8(s,d); }
    BLEND_MODE(screen)   { return s + d - s*d; }
    BLEND_MODE(xor_)     { return s*inv(da) + d*inv(sa); }
    #undef BLEND_MODE

    // These next blend modes apply the same logic to r,g,b, and srcover to alpha.
    #define BLEND_MODE(name)                       \
        SI V name##_channel(V s, V d, V sa, V da); \
        STAGE(name) {                              \
            r = name##_channel(r,dr,a,da);         \
            g = name##_channel(g,dg,a,da);         \
            b = name##_channel(b,db,a,da);         \
            a = a + da*inv(a);                     \
        }                                          \
        SI V name##_channel(V s, V d, V sa, V da)

    BLEND_MODE(darken)     { return s + (d - max(s*da, d*sa)) ; }
    BLEND_MODE(lighten)    { return s + (d - min(s*da, d*sa)) ; }
    BLEND_MODE(difference) { return (s - min(s*da, d*sa)) + (d - min(s*da, d*sa)); }
    BLEND_MODE(exclusion)  { return (s - s*d) + (d - s*d); }

    // These two modes are very sensitive to rounding, so we do them entirely in 16-bit.
    // This is written for hardlight, and overlay is the same swapping src and dst.
    SI V hardlight_or_overlay(V s, V sa, V d, V da) {
        return mul_wide(s, inv(da)) + mul_wide(d, inv(sa))
             + if_then_else(vmovl_s8(s <= sa-s), 2*mul_wide(s,d)
                                               , mul_wide(sa,da) - 2*mul_wide(sa-s, da-d));
    }
    BLEND_MODE(hardlight) { return hardlight_or_overlay(s,sa,d,da); }
    BLEND_MODE(overlay  ) { return hardlight_or_overlay(d,da,s,sa); }

#endif
