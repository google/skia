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
        using U16   = uint16_t __attribute__((ext_vector_type(16)));
        using U32   = uint32_t __attribute__((ext_vector_type(16)));
        using U8x4  = uint8_t  __attribute__((ext_vector_type(64)));
        using U16x4 = uint16_t __attribute__((ext_vector_type(64)));
        using R     = uint8_t  __attribute__((ext_vector_type(32)));
    #else
        using U8    = uint8_t  __attribute__((ext_vector_type( 8)));
        using U16   = uint16_t __attribute__((ext_vector_type( 8)));
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

    STAGE(store_565) {
        auto ptr = ptr_at_xy<uint16_t>(ctx, x,y);

        U32 r = (src.u32 & 0x0000F8) << 8;
        U32 g = (src.u32 & 0x00FC00) >> 5;
        U32 b = (src.u32 & 0xF80000) >> 19;

        store(ptr, __builtin_convertvector(r | g | b, U16), tail);
    }
    STAGE(load_565_dst) {
        auto ptr = ptr_at_xy<const uint16_t>(ctx, x,y);
        U32 p = __builtin_convertvector(load<U16>(ptr, tail), U32);

        U32 rb = ((p & 0xF800) >> 8) | ((p & 0x001F) << 19);
            rb = rb | ((rb >> 5) & 0x70007);
        U32 g = ((p & 0x07E0) << 5) | ((p & 0x0C00) >> 1);

        dst = (0xFF << 24) | rb | g;
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
#endif
