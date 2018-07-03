/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Intentionally NO #pragma once... included multiple times.

// This file is included from skcms.c with some values and types pre-defined:
//    N:    depth of all vectors, 1,4,8, or 16
//
//    F:    a vector of N float
//    I32:  a vector of N int32_t
//    U64:  a vector of N uint64_t
//    U32:  a vector of N uint32_t
//    U16:  a vector of N uint16_t
//    U8:   a vector of N uint8_t
//
//    F0: a vector of N floats set to zero
//    F1: a vector of N floats set to one
//
//    NS(id): a macro that returns unique identifiers
//    ATTR:   an __attribute__ to apply to functions

#if defined(__ARM_FEATURE_FP16_VECTOR_ARITHMETIC)
    // TODO(mtklein): this build supports FP16 compute
#endif

#if defined(__ARM_NEON)
    #include <arm_neon.h>
#elif defined(__SSE__)
    #include <immintrin.h>
#endif

#if N == 4 && defined(__ARM_NEON)
    #define USING_NEON
    #if __ARM_FP & 2
        #define USING_NEON_F16C
    #endif
#elif N == 8 && defined(__AVX__)
    #if defined(__F16C__)
        #define USING_AVX_F16C
    #endif
#endif

// These -Wvector-conversion warnings seem to trigger in very bogus situations,
// like vst3q_f32() expecting a 16x char rather than a 4x float vector.  :/
#if defined(USING_NEON) && defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wvector-conversion"
#endif

// We tag most helper functions as SI, to enforce good code generation
// but also work around what we think is a bug in GCC: when targeting 32-bit
// x86, GCC tends to pass U16 (4x uint16_t vector) function arguments in the
// MMX mm0 register, which seems to mess with unrelated code that later uses
// x87 FP instructions (MMX's mm0 is an alias for x87's st0 register).
//
// It helps codegen to call __builtin_memcpy() when we know the byte count at compile time.
#if defined(__clang__) || defined(__GNUC__)
    #define SI static inline __attribute__((always_inline))
    #define small_memcpy __builtin_memcpy
#else
    #define SI static inline
    #define small_memcpy memcpy
#endif

// (T)v is a cast when N == 1 and a bit-pun when N>1, so we must use CAST(T, v) to actually cast.
#if N == 1
    #define CAST(T, v) (T)(v)
#elif defined(__clang__)
    #define CAST(T, v) __builtin_convertvector((v), T)
#elif N == 4
    #define CAST(T, v) T{(v)[0],(v)[1],(v)[2],(v)[3]}
#elif N == 8
    #define CAST(T, v) T{(v)[0],(v)[1],(v)[2],(v)[3], (v)[4],(v)[5],(v)[6],(v)[7]}
#elif N == 16
    #define CAST(T, v) T{(v)[0],(v)[1],(v)[ 2],(v)[ 3], (v)[ 4],(v)[ 5],(v)[ 6],(v)[ 7], \
                         (v)[8],(v)[9],(v)[10],(v)[11], (v)[12],(v)[13],(v)[14],(v)[15]}
#endif

// When we convert from float to fixed point, it's very common to want to round,
// and for some reason compilers generate better code when converting to int32_t.
// To serve both those ends, we use this function to_fixed() instead of direct CASTs.
SI ATTR I32 NS(to_fixed_)(F f) {  return CAST(I32, f + 0.5f); }
#define to_fixed NS(to_fixed_)

// Comparisons result in bool when N == 1, in an I32 mask when N > 1.
// We've made this a macro so it can be type-generic...
// always (T) cast the result to the type you expect the result to be.
#if N == 1
    #define if_then_else(c,t,e) ( (c) ? (t) : (e) )
#else
    #define if_then_else(c,t,e) ( ((c) & (I32)(t)) | (~(c) & (I32)(e)) )
#endif

#if defined(USING_NEON_F16C)
    SI ATTR F   NS(F_from_Half_(U16 half)) { return      vcvt_f32_f16((float16x4_t)half); }
    SI ATTR U16 NS(Half_from_F_(F      f)) { return (U16)vcvt_f16_f32(                f); }
#elif defined(__AVX512F__)
    SI ATTR F   NS(F_from_Half_)(U16 half) { return (F)_mm512_cvtph_ps((__m256i)half); }
    SI ATTR U16 NS(Half_from_F_)(F f) {
        return (U16)_mm512_cvtps_ph((__m512 )f, _MM_FROUND_CUR_DIRECTION );
    }
#elif defined(USING_AVX_F16C)
    SI ATTR F NS(F_from_Half_)(U16 half) {
        typedef int16_t __attribute__((vector_size(16))) I16;
        return __builtin_ia32_vcvtph2ps256((I16)half);
    }
    SI ATTR U16 NS(Half_from_F_)(F f) {
        return (U16)__builtin_ia32_vcvtps2ph256(f, 0x04/*_MM_FROUND_CUR_DIRECTION*/);
    }
#else
    SI ATTR F NS(F_from_Half_)(U16 half) {
        U32 wide = CAST(U32, half);
        // A half is 1-5-10 sign-exponent-mantissa, with 15 exponent bias.
        U32 s  = wide & 0x8000,
            em = wide ^ s;

        // Constructing the float is easy if the half is not denormalized.
        U32 norm_bits = (s<<16) + (em<<13) + ((127-15)<<23);
        F norm;
        small_memcpy(&norm, &norm_bits, sizeof(norm));

        // Simply flush all denorm half floats to zero.
        return (F)if_then_else(em < 0x0400, F0, norm);
    }

    SI ATTR U16 NS(Half_from_F_)(F f) {
        // A float is 1-8-23 sign-exponent-mantissa, with 127 exponent bias.
        U32 sem;
        small_memcpy(&sem, &f, sizeof(sem));

        U32 s  = sem & 0x80000000,
            em = sem ^ s;

        // For simplicity we flush denorm half floats (including all denorm floats) to zero.
        return CAST(U16, (U32)if_then_else(em < 0x38800000, (U32)F0
                                                          , (s>>16) + (em>>13) - ((127-15)<<10)));
    }
#endif

#define F_from_Half NS(F_from_Half_)
#define Half_from_F NS(Half_from_F_)

// Swap high and low bytes of 16-bit lanes, converting between big-endian and little-endian.
#if defined(USING_NEON)
    SI ATTR U16 NS(swap_endian_16_)(U16 v) {
        return (U16)vrev16_u8((uint8x8_t) v);
    }
    #define swap_endian_16 NS(swap_endian_16_)
#endif

// Passing by U64* instead of U64 avoids ABI warnings.  It's all moot when inlined.
SI ATTR void NS(swap_endian_16x4_)(U64* rgba) {
    *rgba = (*rgba & 0x00ff00ff00ff00ff) << 8
          | (*rgba & 0xff00ff00ff00ff00) >> 8;
}
#define swap_endian_16x4 NS(swap_endian_16x4_)

#if defined(USING_NEON)
    SI ATTR F NS(min__)(F x, F y) { return (F)vminq_f32((float32x4_t)x, (float32x4_t)y); }
    SI ATTR F NS(max__)(F x, F y) { return (F)vmaxq_f32((float32x4_t)x, (float32x4_t)y); }
#else
    SI ATTR F NS(min__)(F x, F y) { return (F)if_then_else(x > y, y, x); }
    SI ATTR F NS(max__)(F x, F y) { return (F)if_then_else(x < y, y, x); }
#endif

#define min_ NS(min__)
#define max_ NS(max__)

SI ATTR F NS(floor__)(F x) {
#if N == 1
    return floorf_(x);
#elif defined(__aarch64__)
    return vrndmq_f32(x);
#elif defined(__AVX512F__)
    return _mm512_floor_ps(x);
#elif defined(__AVX__)
    return __builtin_ia32_roundps256(x, 0x01/*_MM_FROUND_FLOOR*/);
#elif defined(__SSE4_1__)
    return _mm_floor_ps(x);
#else
    // Round trip through integers with a truncating cast.
    F roundtrip = CAST(F, CAST(I32, x));
    // If x is negative, truncating gives the ceiling instead of the floor.
    return roundtrip - (F)if_then_else(roundtrip > x, F1, F0);

    // This implementation fails for values of x that are outside
    // the range an integer can represent.  We expect most x to be small.
#endif
}
#define floor_ NS(floor__)

SI ATTR F NS(approx_log2_)(F x) {
    // The first approximation of log2(x) is its exponent 'e', minus 127.
    I32 bits;
    small_memcpy(&bits, &x, sizeof(bits));

    F e = CAST(F, bits) * (1.0f / (1<<23));

    // If we use the mantissa too we can refine the error signficantly.
    I32 m_bits = (bits & 0x007fffff) | 0x3f000000;
    F m;
    small_memcpy(&m, &m_bits, sizeof(m));

    return e - 124.225514990f
             -   1.498030302f*m
             -   1.725879990f/(0.3520887068f + m);
}
#define approx_log2 NS(approx_log2_)

SI ATTR F NS(approx_exp2_)(F x) {
    F fract = x - floor_(x);

    I32 bits = CAST(I32, (1.0f * (1<<23)) * (x + 121.274057500f
                                               -   1.490129070f*fract
                                               +  27.728023300f/(4.84252568f - fract)));
    small_memcpy(&x, &bits, sizeof(x));
    return x;
}
#define approx_exp2 NS(approx_exp2_)

SI ATTR F NS(approx_pow_)(F x, float y) {
    return (F)if_then_else((x == F0) | (x == F1), x
                                                , approx_exp2(approx_log2(x) * y));
}
#define approx_pow NS(approx_pow_)

// Return tf(x).
SI ATTR F NS(apply_tf_)(const skcms_TransferFunction* tf, F x) {
    F sign = (F)if_then_else(x < 0, -F1, F1);
    x *= sign;

    F linear    =            tf->c*x + tf->f;
    F nonlinear = approx_pow(tf->a*x + tf->b, tf->g) + tf->e;

    return sign * (F)if_then_else(x < tf->d, linear, nonlinear);
}
#define apply_tf NS(apply_tf_)

// Strided loads and stores of N values, starting from p.
#if N == 1
    #define LOAD_3(T, p) (T)(p)[0]
    #define LOAD_4(T, p) (T)(p)[0]
    #define STORE_3(p, v) (p)[0] = v
    #define STORE_4(p, v) (p)[0] = v
#elif N == 4 && !defined(USING_NEON)
    #define LOAD_3(T, p) T{(p)[0], (p)[3], (p)[6], (p)[ 9]}
    #define LOAD_4(T, p) T{(p)[0], (p)[4], (p)[8], (p)[12]};
    #define STORE_3(p, v) (p)[0] = (v)[0]; (p)[3] = (v)[1]; (p)[6] = (v)[2]; (p)[ 9] = (v)[3]
    #define STORE_4(p, v) (p)[0] = (v)[0]; (p)[4] = (v)[1]; (p)[8] = (v)[2]; (p)[12] = (v)[3]
#elif N == 8
    #define LOAD_3(T, p) T{(p)[0], (p)[3], (p)[6], (p)[ 9],  (p)[12], (p)[15], (p)[18], (p)[21]}
    #define LOAD_4(T, p) T{(p)[0], (p)[4], (p)[8], (p)[12],  (p)[16], (p)[20], (p)[24], (p)[28]}
    #define STORE_3(p, v) (p)[ 0] = (v)[0]; (p)[ 3] = (v)[1]; (p)[ 6] = (v)[2]; (p)[ 9] = (v)[3]; \
                          (p)[12] = (v)[4]; (p)[15] = (v)[5]; (p)[18] = (v)[6]; (p)[21] = (v)[7]
    #define STORE_4(p, v) (p)[ 0] = (v)[0]; (p)[ 4] = (v)[1]; (p)[ 8] = (v)[2]; (p)[12] = (v)[3]; \
                          (p)[16] = (v)[4]; (p)[20] = (v)[5]; (p)[24] = (v)[6]; (p)[28] = (v)[7]
#elif N == 16
    // TODO: revisit with AVX-512 gathers and scatters?
    #define LOAD_3(T, p) T{(p)[ 0], (p)[ 3], (p)[ 6], (p)[ 9], \
                           (p)[12], (p)[15], (p)[18], (p)[21], \
                           (p)[24], (p)[27], (p)[30], (p)[33], \
                           (p)[36], (p)[39], (p)[42], (p)[45]}

    #define LOAD_4(T, p) T{(p)[ 0], (p)[ 4], (p)[ 8], (p)[12], \
                           (p)[16], (p)[20], (p)[24], (p)[28], \
                           (p)[32], (p)[36], (p)[40], (p)[44], \
                           (p)[48], (p)[52], (p)[56], (p)[60]}

    #define STORE_3(p, v) \
        (p)[ 0] = (v)[ 0]; (p)[ 3] = (v)[ 1]; (p)[ 6] = (v)[ 2]; (p)[ 9] = (v)[ 3]; \
        (p)[12] = (v)[ 4]; (p)[15] = (v)[ 5]; (p)[18] = (v)[ 6]; (p)[21] = (v)[ 7]; \
        (p)[24] = (v)[ 8]; (p)[27] = (v)[ 9]; (p)[30] = (v)[10]; (p)[33] = (v)[11]; \
        (p)[36] = (v)[12]; (p)[39] = (v)[13]; (p)[42] = (v)[14]; (p)[45] = (v)[15]

    #define STORE_4(p, v) \
        (p)[ 0] = (v)[ 0]; (p)[ 4] = (v)[ 1]; (p)[ 8] = (v)[ 2]; (p)[12] = (v)[ 3]; \
        (p)[16] = (v)[ 4]; (p)[20] = (v)[ 5]; (p)[24] = (v)[ 6]; (p)[28] = (v)[ 7]; \
        (p)[32] = (v)[ 8]; (p)[36] = (v)[ 9]; (p)[40] = (v)[10]; (p)[44] = (v)[11]; \
        (p)[48] = (v)[12]; (p)[52] = (v)[13]; (p)[56] = (v)[14]; (p)[60] = (v)[15]
#endif

SI ATTR U8 NS(gather_8_)(const uint8_t* p, I32 ix) {
#if N == 1
    U8 v = p[ix];
#elif N == 4
    U8 v = { p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]] };
#elif N == 8
    U8 v = { p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]],
             p[ix[4]], p[ix[5]], p[ix[6]], p[ix[7]] };
#elif N == 16
    U8 v = { p[ix[ 0]], p[ix[ 1]], p[ix[ 2]], p[ix[ 3]],
             p[ix[ 4]], p[ix[ 5]], p[ix[ 6]], p[ix[ 7]],
             p[ix[ 8]], p[ix[ 9]], p[ix[10]], p[ix[11]],
             p[ix[12]], p[ix[13]], p[ix[14]], p[ix[15]] };
#endif
    return v;
}
#define gather_8 NS(gather_8_)

// Helper for gather_16(), loading the ix'th 16-bit value from p.
SI ATTR uint16_t NS(load_16_)(const uint8_t* p, int ix) {
    uint16_t v;
    small_memcpy(&v, p + 2*ix, 2);
    return v;
}
#define load_16 NS(load_16_)

SI ATTR U16 NS(gather_16_)(const uint8_t* p, I32 ix) {
#if N == 1
    U16 v = load_16(p,ix);
#elif N == 4
    U16 v = { load_16(p,ix[0]), load_16(p,ix[1]), load_16(p,ix[2]), load_16(p,ix[3]) };
#elif N == 8
    U16 v = { load_16(p,ix[0]), load_16(p,ix[1]), load_16(p,ix[2]), load_16(p,ix[3]),
              load_16(p,ix[4]), load_16(p,ix[5]), load_16(p,ix[6]), load_16(p,ix[7]) };
#elif N == 16
    U16 v = { load_16(p,ix[ 0]), load_16(p,ix[ 1]), load_16(p,ix[ 2]), load_16(p,ix[ 3]),
              load_16(p,ix[ 4]), load_16(p,ix[ 5]), load_16(p,ix[ 6]), load_16(p,ix[ 7]),
              load_16(p,ix[ 8]), load_16(p,ix[ 9]), load_16(p,ix[10]), load_16(p,ix[11]),
              load_16(p,ix[12]), load_16(p,ix[13]), load_16(p,ix[14]), load_16(p,ix[15]) };
#endif
    return v;
}
#define gather_16 NS(gather_16_)

#if !defined(__AVX2__)
    // Helpers for gather_24/48(), loading the ix'th 24/48-bit value from p, and 1/2 extra bytes.
    SI ATTR uint32_t NS(load_24_32_)(const uint8_t* p, int ix) {
        uint32_t v;
        small_memcpy(&v, p + 3*ix, 4);
        return v;
    }
    SI ATTR uint64_t NS(load_48_64_)(const uint8_t* p, int ix) {
        uint64_t v;
        small_memcpy(&v, p + 6*ix, 8);
        return v;
    }
    #define load_24_32 NS(load_24_32_)
    #define load_48_64 NS(load_48_64_)
#endif

SI ATTR U32 NS(gather_24_)(const uint8_t* p, I32 ix) {
    // First, back up a byte.  Any place we're gathering from has a safe junk byte to read
    // in front of it, either a previous table value, or some tag metadata.
    p -= 1;

    // Now load multiples of 4 bytes (a junk byte, then r,g,b).
#if N == 1
    U32 v = load_24_32(p,ix);
#elif N == 4
    U32 v = { load_24_32(p,ix[0]), load_24_32(p,ix[1]), load_24_32(p,ix[2]), load_24_32(p,ix[3]) };
#elif N == 8 && !defined(__AVX2__)
    U32 v = { load_24_32(p,ix[0]), load_24_32(p,ix[1]), load_24_32(p,ix[2]), load_24_32(p,ix[3]),
              load_24_32(p,ix[4]), load_24_32(p,ix[5]), load_24_32(p,ix[6]), load_24_32(p,ix[7]) };
#elif N == 8
    // The gather instruction here doesn't need any particular alignment,
    // but the intrinsic takes a const int*.
    const int* p4;
    small_memcpy(&p4, &p, sizeof(p4));
    I32 zero = { 0, 0, 0, 0,  0, 0, 0, 0},
        mask = {-1,-1,-1,-1, -1,-1,-1,-1};
    #if defined(__clang__)
        U32 v = (U32)__builtin_ia32_gatherd_d256(zero, p4, 3*ix, mask, 1);
    #elif defined(__GNUC__)
        U32 v = (U32)__builtin_ia32_gathersiv8si(zero, p4, 3*ix, mask, 1);
    #endif
#elif N == 16
    // The intrinsic is supposed to take const void* now, but it takes const int*, just like AVX2.
    // And AVX-512 swapped the order of arguments.  :/
    const int* p4;
    small_memcpy(&p4, &p, sizeof(p4));
    U32 v = (U32)_mm512_i32gather_epi32((__m512i)(3*ix), p4, 1);
#endif

    // Shift off the junk byte, leaving r,g,b in low 24 bits (and zero in the top 8).
    return v >> 8;
}
#define gather_24 NS(gather_24_)

#if !defined(__arm__)
    SI ATTR void NS(gather_48_)(const uint8_t* p, I32 ix, U64* v) {
        // As in gather_24(), with everything doubled.
        p -= 2;

    #if N == 1
        *v = load_48_64(p,ix);
    #elif N == 4
        *v = U64{
            load_48_64(p,ix[0]), load_48_64(p,ix[1]), load_48_64(p,ix[2]), load_48_64(p,ix[3]),
        };
    #elif N == 8 && !defined(__AVX2__)
        *v = U64{
            load_48_64(p,ix[0]), load_48_64(p,ix[1]), load_48_64(p,ix[2]), load_48_64(p,ix[3]),
            load_48_64(p,ix[4]), load_48_64(p,ix[5]), load_48_64(p,ix[6]), load_48_64(p,ix[7]),
        };
    #elif N == 8
        typedef int32_t   __attribute__((vector_size(16))) Half_I32;
        typedef long long __attribute__((vector_size(32))) Half_I64;

        // The gather instruction here doesn't need any particular alignment,
        // but the intrinsic takes a const long long*.
        const long long int* p8;
        small_memcpy(&p8, &p, sizeof(p8));

        Half_I64 zero = { 0, 0, 0, 0},
                 mask = {-1,-1,-1,-1};

        ix *= 6;
        Half_I32 ix_lo = { ix[0], ix[1], ix[2], ix[3] },
                 ix_hi = { ix[4], ix[5], ix[6], ix[7] };

        #if defined(__clang__)
            Half_I64 lo = (Half_I64)__builtin_ia32_gatherd_q256(zero, p8, ix_lo, mask, 1),
                     hi = (Half_I64)__builtin_ia32_gatherd_q256(zero, p8, ix_hi, mask, 1);
        #elif defined(__GNUC__)
            Half_I64 lo = (Half_I64)__builtin_ia32_gathersiv4di(zero, p8, ix_lo, mask, 1),
                     hi = (Half_I64)__builtin_ia32_gathersiv4di(zero, p8, ix_hi, mask, 1);
        #endif
        small_memcpy((char*)v +  0, &lo, 32);
        small_memcpy((char*)v + 32, &hi, 32);
    #elif N == 16
        const long long int* p8;
        small_memcpy(&p8, &p, sizeof(p8));
        __m512i lo = _mm512_i32gather_epi64(_mm512_extracti32x8_epi32((__m512i)(6*ix), 0), p8, 1),
                hi = _mm512_i32gather_epi64(_mm512_extracti32x8_epi32((__m512i)(6*ix), 1), p8, 1);
        small_memcpy((char*)v +  0, &lo, 64);
        small_memcpy((char*)v + 64, &hi, 64);
    #endif

        *v >>= 16;
    }
    #define gather_48 NS(gather_48_)
#endif

SI ATTR F NS(F_from_U8_)(U8 v) {
    return CAST(F, v) * (1/255.0f);
}
#define F_from_U8 NS(F_from_U8_)

SI ATTR F NS(F_from_U16_BE_)(U16 v) {
    // All 16-bit ICC values are big-endian, so we byte swap before converting to float.
    // MSVC catches the "loss" of data here in the portable path, so we also make sure to mask.
    v = (U16)( ((v<<8)|(v>>8)) & 0xffff );
    return CAST(F, v) * (1/65535.0f);
}
#define F_from_U16_BE NS(F_from_U16_BE_)

SI ATTR F NS(minus_1_ulp_)(F v) {
    I32 bits;
    small_memcpy(&bits, &v, sizeof(bits));
    bits = bits - 1;
    small_memcpy(&v, &bits, sizeof(bits));
    return v;
}
#define minus_1_ulp NS(minus_1_ulp_)

SI ATTR F NS(table_8_)(const skcms_Curve* curve, F v) {
    // Clamp the input to [0,1], then scale to a table index.
    F ix = max_(F0, min_(v, F1)) * (float)(curve->table_entries - 1);

    // We'll look up (equal or adjacent) entries at lo and hi, then lerp by t between the two.
    I32 lo = CAST(I32,             ix      ),
        hi = CAST(I32, minus_1_ulp(ix+1.0f));
    F t = ix - CAST(F, lo);  // i.e. the fractional part of ix.

    // TODO: can we load l and h simultaneously?  Each entry in 'h' is either
    // the same as in 'l' or adjacent.  We have a rough idea that's it'd always be safe
    // to read adjacent entries and perhaps underflow the table by a byte or two
    // (it'd be junk, but always safe to read).  Not sure how to lerp yet.
    F l = F_from_U8(gather_8(curve->table_8, lo)),
      h = F_from_U8(gather_8(curve->table_8, hi));
    return l + (h-l)*t;
}

SI ATTR F NS(table_16_)(const skcms_Curve* curve, F v) {
    // All just as in table_8() until the gathers.
    F ix = max_(F0, min_(v, F1)) * (float)(curve->table_entries - 1);

    I32 lo = CAST(I32,             ix      ),
        hi = CAST(I32, minus_1_ulp(ix+1.0f));
    F t = ix - CAST(F, lo);

    // TODO: as above, load l and h simultaneously?
    // Here we could even use AVX2-style 32-bit gathers.
    F l = F_from_U16_BE(gather_16(curve->table_16, lo)),
      h = F_from_U16_BE(gather_16(curve->table_16, hi));
    return l + (h-l)*t;
}

// Color lookup tables, by input dimension and bit depth.
SI ATTR void NS(clut_0_8_)(const skcms_A2B* a2b, I32 ix, I32 stride, F* r, F* g, F* b, F a) {
    U32 rgb = gather_24(a2b->grid_8, ix);

    *r = CAST(F, (rgb >>  0) & 0xff) * (1/255.0f);
    *g = CAST(F, (rgb >>  8) & 0xff) * (1/255.0f);
    *b = CAST(F, (rgb >> 16) & 0xff) * (1/255.0f);

    (void)a;
    (void)stride;
}
SI ATTR void NS(clut_0_16_)(const skcms_A2B* a2b, I32 ix, I32 stride, F* r, F* g, F* b, F a) {
    #if defined(__arm__)
        // This is up to 2x faster on 32-bit ARM than the #else-case fast path.
        *r = F_from_U16_BE(gather_16(a2b->grid_16, 3*ix+0));
        *g = F_from_U16_BE(gather_16(a2b->grid_16, 3*ix+1));
        *b = F_from_U16_BE(gather_16(a2b->grid_16, 3*ix+2));
    #else
        // This strategy is much faster for 64-bit builds, and fine for 32-bit x86 too.
        U64 rgb;
        gather_48(a2b->grid_16, ix, &rgb);
        swap_endian_16x4(&rgb);

        *r = CAST(F, (rgb >>  0) & 0xffff) * (1/65535.0f);
        *g = CAST(F, (rgb >> 16) & 0xffff) * (1/65535.0f);
        *b = CAST(F, (rgb >> 32) & 0xffff) * (1/65535.0f);
    #endif
    (void)a;
    (void)stride;
}

// __attribute__((always_inline)) hits some pathological case in GCC that makes
// compilation way too slow for my patience.
#if defined(__clang__)
    #define MAYBE_SI SI
#else
    #define MAYBE_SI static inline
#endif

// These are all the same basic approach: handle one dimension, then the rest recursively.
// We let "I" be the current dimension, and "J" the previous dimension, I-1.  "B" is the bit depth.
#define DEF_CLUT(I,J,B)                                                                           \
    MAYBE_SI ATTR                                                                                 \
    void NS(clut_##I##_##B##_)(const skcms_A2B* a2b, I32 ix, I32 stride, F* r, F* g, F* b, F a) { \
        I32 limit = CAST(I32, F0);                                                                \
        limit += a2b->grid_points[I-1];                                                           \
                                                                                                  \
        const F* srcs[] = { r,g,b,&a };                                                           \
        F src = *srcs[I-1];                                                                       \
                                                                                                  \
        F x = max_(F0, min_(src, F1)) * CAST(F, limit - 1);                                       \
                                                                                                  \
        I32 lo = CAST(I32,             x      ),                                                  \
            hi = CAST(I32, minus_1_ulp(x+1.0f));                                                  \
        F lr = *r, lg = *g, lb = *b,                                                              \
          hr = *r, hg = *g, hb = *b;                                                              \
        NS(clut_##J##_##B##_)(a2b, stride*lo + ix, stride*limit, &lr,&lg,&lb,a);                  \
        NS(clut_##J##_##B##_)(a2b, stride*hi + ix, stride*limit, &hr,&hg,&hb,a);                  \
                                                                                                  \
        F t = x - CAST(F, lo);                                                                    \
        *r = lr + (hr-lr)*t;                                                                      \
        *g = lg + (hg-lg)*t;                                                                      \
        *b = lb + (hb-lb)*t;                                                                      \
    }

DEF_CLUT(1,0,8)
DEF_CLUT(2,1,8)
DEF_CLUT(3,2,8)
DEF_CLUT(4,3,8)

DEF_CLUT(1,0,16)
DEF_CLUT(2,1,16)
DEF_CLUT(3,2,16)
DEF_CLUT(4,3,16)

ATTR
static void NS(exec_ops)(const Op* ops, const void** args,
                         const char* src, char* dst, int i) {
    F r = F0, g = F0, b = F0, a = F0;
    while (true) {
        switch (*ops++) {
            case Op_noop: break;

            case Op_load_a8:{
                U8 alpha;
                small_memcpy(&alpha, src + i, N);
                a = F_from_U8(alpha);
            } break;

            case Op_load_g8:{
                U8 gray;
                small_memcpy(&gray, src + i, N);
                r = g = b = F_from_U8(gray);
            } break;

            case Op_load_4444:{
                U16 abgr;
                small_memcpy(&abgr, src + 2*i, 2*N);

                r = CAST(F, (abgr >> 12) & 0xf) * (1/15.0f);
                g = CAST(F, (abgr >>  8) & 0xf) * (1/15.0f);
                b = CAST(F, (abgr >>  4) & 0xf) * (1/15.0f);
                a = CAST(F, (abgr >>  0) & 0xf) * (1/15.0f);
            } break;

            case Op_load_565:{
                U16 rgb;
                small_memcpy(&rgb, src + 2*i, 2*N);

                r = CAST(F, rgb & (uint16_t)(31<< 0)) * (1.0f / (31<< 0));
                g = CAST(F, rgb & (uint16_t)(63<< 5)) * (1.0f / (63<< 5));
                b = CAST(F, rgb & (uint16_t)(31<<11)) * (1.0f / (31<<11));
                a = F1;
            } break;

            case Op_load_888:{
                const uint8_t* rgb = (const uint8_t*)(src + 3*i);
            #if defined(USING_NEON)
                // There's no uint8x4x3_t or vld3 load for it, so we'll load each rgb pixel one at
                // a time.  Since we're doing that, we might as well load them into 16-bit lanes.
                // (We'd even load into 32-bit lanes, but that's not possible on ARMv7.)
                uint8x8x3_t v = {{ vdup_n_u8(0), vdup_n_u8(0), vdup_n_u8(0) }};
                v = vld3_lane_u8(rgb+0, v, 0);
                v = vld3_lane_u8(rgb+3, v, 2);
                v = vld3_lane_u8(rgb+6, v, 4);
                v = vld3_lane_u8(rgb+9, v, 6);

                // Now if we squint, those 3 uint8x8_t we constructed are really U16s, easy to
                // convert to F.  (Again, U32 would be even better here if drop ARMv7 or split
                // ARMv7 and ARMv8 impls.)
                r = CAST(F, (U16)v.val[0]) * (1/255.0f);
                g = CAST(F, (U16)v.val[1]) * (1/255.0f);
                b = CAST(F, (U16)v.val[2]) * (1/255.0f);
            #else
                r = CAST(F, LOAD_3(U32, rgb+0) ) * (1/255.0f);
                g = CAST(F, LOAD_3(U32, rgb+1) ) * (1/255.0f);
                b = CAST(F, LOAD_3(U32, rgb+2) ) * (1/255.0f);
            #endif
                a = F1;
            } break;

            case Op_load_8888:{
                U32 rgba;
                small_memcpy(&rgba, src + 4*i, 4*N);

                r = CAST(F, (rgba >>  0) & 0xff) * (1/255.0f);
                g = CAST(F, (rgba >>  8) & 0xff) * (1/255.0f);
                b = CAST(F, (rgba >> 16) & 0xff) * (1/255.0f);
                a = CAST(F, (rgba >> 24) & 0xff) * (1/255.0f);
            } break;

            case Op_load_1010102:{
                U32 rgba;
                small_memcpy(&rgba, src + 4*i, 4*N);

                r = CAST(F, (rgba >>  0) & 0x3ff) * (1/1023.0f);
                g = CAST(F, (rgba >> 10) & 0x3ff) * (1/1023.0f);
                b = CAST(F, (rgba >> 20) & 0x3ff) * (1/1023.0f);
                a = CAST(F, (rgba >> 30) & 0x3  ) * (1/   3.0f);
            } break;

            case Op_load_161616:{
                uintptr_t ptr = (uintptr_t)(src + 6*i);
                assert( (ptr & 1) == 0 );                   // src must be 2-byte aligned for this
                const uint16_t* rgb = (const uint16_t*)ptr; // cast to const uint16_t* to be safe.
            #if defined(USING_NEON)
                uint16x4x3_t v = vld3_u16(rgb);
                r = CAST(F, swap_endian_16((U16)v.val[0])) * (1/65535.0f);
                g = CAST(F, swap_endian_16((U16)v.val[1])) * (1/65535.0f);
                b = CAST(F, swap_endian_16((U16)v.val[2])) * (1/65535.0f);
            #else
                U32 R = LOAD_3(U32, rgb+0),
                    G = LOAD_3(U32, rgb+1),
                    B = LOAD_3(U32, rgb+2);
                // R,G,B are big-endian 16-bit, so byte swap them before converting to float.
                r = CAST(F, (R & 0x00ff)<<8 | (R & 0xff00)>>8) * (1/65535.0f);
                g = CAST(F, (G & 0x00ff)<<8 | (G & 0xff00)>>8) * (1/65535.0f);
                b = CAST(F, (B & 0x00ff)<<8 | (B & 0xff00)>>8) * (1/65535.0f);
            #endif
                a = F1;
            } break;

            case Op_load_16161616:{
                uintptr_t ptr = (uintptr_t)(src + 8*i);
                assert( (ptr & 1) == 0 );                    // src must be 2-byte aligned for this
                const uint16_t* rgba = (const uint16_t*)ptr; // cast to const uint16_t* to be safe.
            #if defined(USING_NEON)
                uint16x4x4_t v = vld4_u16(rgba);
                r = CAST(F, swap_endian_16((U16)v.val[0])) * (1/65535.0f);
                g = CAST(F, swap_endian_16((U16)v.val[1])) * (1/65535.0f);
                b = CAST(F, swap_endian_16((U16)v.val[2])) * (1/65535.0f);
                a = CAST(F, swap_endian_16((U16)v.val[3])) * (1/65535.0f);
            #else
                U64 px;
                small_memcpy(&px, rgba, 8*N);

                swap_endian_16x4(&px);
                r = CAST(F, (px >>  0) & 0xffff) * (1/65535.0f);
                g = CAST(F, (px >> 16) & 0xffff) * (1/65535.0f);
                b = CAST(F, (px >> 32) & 0xffff) * (1/65535.0f);
                a = CAST(F, (px >> 48) & 0xffff) * (1/65535.0f);
            #endif
            } break;

            case Op_load_hhh:{
                uintptr_t ptr = (uintptr_t)(src + 6*i);
                assert( (ptr & 1) == 0 );                   // src must be 2-byte aligned for this
                const uint16_t* rgb = (const uint16_t*)ptr; // cast to const uint16_t* to be safe.
            #if defined(USING_NEON)
                uint16x4x3_t v = vld3_u16(rgb);
                U16 R = (U16)v.val[0],
                    G = (U16)v.val[1],
                    B = (U16)v.val[2];
            #else
                U16 R = LOAD_3(U16, rgb+0),
                    G = LOAD_3(U16, rgb+1),
                    B = LOAD_3(U16, rgb+2);
            #endif
                r = F_from_Half(R);
                g = F_from_Half(G);
                b = F_from_Half(B);
                a = F1;
            } break;

            case Op_load_hhhh:{
                uintptr_t ptr = (uintptr_t)(src + 8*i);
                assert( (ptr & 1) == 0 );                    // src must be 2-byte aligned for this
                const uint16_t* rgba = (const uint16_t*)ptr; // cast to const uint16_t* to be safe.
            #if defined(USING_NEON)
                uint16x4x4_t v = vld4_u16(rgba);
                U16 R = (U16)v.val[0],
                    G = (U16)v.val[1],
                    B = (U16)v.val[2],
                    A = (U16)v.val[3];
            #else
                U64 px;
                small_memcpy(&px, rgba, 8*N);
                U16 R = CAST(U16, (px >>  0) & 0xffff),
                    G = CAST(U16, (px >> 16) & 0xffff),
                    B = CAST(U16, (px >> 32) & 0xffff),
                    A = CAST(U16, (px >> 48) & 0xffff);
            #endif
                r = F_from_Half(R);
                g = F_from_Half(G);
                b = F_from_Half(B);
                a = F_from_Half(A);
            } break;

            case Op_load_fff:{
                uintptr_t ptr = (uintptr_t)(src + 12*i);
                assert( (ptr & 3) == 0 );                   // src must be 4-byte aligned for this
                const float* rgb = (const float*)ptr;       // cast to const float* to be safe.
            #if defined(USING_NEON)
                float32x4x3_t v = vld3q_f32(rgb);
                r = (F)v.val[0];
                g = (F)v.val[1];
                b = (F)v.val[2];
            #else
                r = LOAD_3(F, rgb+0);
                g = LOAD_3(F, rgb+1);
                b = LOAD_3(F, rgb+2);
            #endif
                a = F1;
            } break;

            case Op_load_ffff:{
                uintptr_t ptr = (uintptr_t)(src + 16*i);
                assert( (ptr & 3) == 0 );                   // src must be 4-byte aligned for this
                const float* rgba = (const float*)ptr;      // cast to const float* to be safe.
            #if defined(USING_NEON)
                float32x4x4_t v = vld4q_f32(rgba);
                r = (F)v.val[0];
                g = (F)v.val[1];
                b = (F)v.val[2];
                a = (F)v.val[3];
            #else
                r = LOAD_4(F, rgba+0);
                g = LOAD_4(F, rgba+1);
                b = LOAD_4(F, rgba+2);
                a = LOAD_4(F, rgba+3);
            #endif
            } break;

            case Op_swap_rb:{
                F t = r;
                r = b;
                b = t;
            } break;

            case Op_clamp:{
                r = max_(F0, min_(r, F1));
                g = max_(F0, min_(g, F1));
                b = max_(F0, min_(b, F1));
                a = max_(F0, min_(a, F1));
            } break;

            case Op_invert:{
                r = F1 - r;
                g = F1 - g;
                b = F1 - b;
                a = F1 - a;
            } break;

            case Op_force_opaque:{
                a = F1;
            } break;

            case Op_premul:{
                r *= a;
                g *= a;
                b *= a;
            } break;

            case Op_unpremul:{
                F scale = (F)if_then_else(F1 / a < INFINITY_, F1 / a, F0);
                r *= scale;
                g *= scale;
                b *= scale;
            } break;

            case Op_matrix_3x3:{
                const skcms_Matrix3x3* matrix = (const skcms_Matrix3x3*) *args++;
                const float* m = &matrix->vals[0][0];

                F R = m[0]*r + m[1]*g + m[2]*b,
                  G = m[3]*r + m[4]*g + m[5]*b,
                  B = m[6]*r + m[7]*g + m[8]*b;

                r = R;
                g = G;
                b = B;
            } break;

            case Op_matrix_3x4:{
                const skcms_Matrix3x4* matrix = (const skcms_Matrix3x4*) *args++;
                const float* m = &matrix->vals[0][0];

                F R = m[0]*r + m[1]*g + m[ 2]*b + m[ 3],
                  G = m[4]*r + m[5]*g + m[ 6]*b + m[ 7],
                  B = m[8]*r + m[9]*g + m[10]*b + m[11];

                r = R;
                g = G;
                b = B;
            } break;

            case Op_lab_to_xyz:{
                // The L*a*b values are in r,g,b, but normalized to [0,1].  Reconstruct them:
                F L = r * 100.0f,
                  A = g * 255.0f - 128.0f,
                  B = b * 255.0f - 128.0f;

                // Convert to CIE XYZ.
                F Y = (L + 16.0f) * (1/116.0f),
                  X = Y + A*(1/500.0f),
                  Z = Y - B*(1/200.0f);

                X = (F)if_then_else(X*X*X > 0.008856f, X*X*X, (X - (16/116.0f)) * (1/7.787f));
                Y = (F)if_then_else(Y*Y*Y > 0.008856f, Y*Y*Y, (Y - (16/116.0f)) * (1/7.787f));
                Z = (F)if_then_else(Z*Z*Z > 0.008856f, Z*Z*Z, (Z - (16/116.0f)) * (1/7.787f));

                // Adjust to XYZD50 illuminant, and stuff back into r,g,b for the next op.
                r = X * 0.9642f;
                g = Y          ;
                b = Z * 0.8249f;
            } break;

            case Op_tf_r:{ r = apply_tf((const skcms_TransferFunction*)*args++, r); } break;
            case Op_tf_g:{ g = apply_tf((const skcms_TransferFunction*)*args++, g); } break;
            case Op_tf_b:{ b = apply_tf((const skcms_TransferFunction*)*args++, b); } break;
            case Op_tf_a:{ a = apply_tf((const skcms_TransferFunction*)*args++, a); } break;

            case Op_table_8_r: { r = NS(table_8_ )((const skcms_Curve*)*args++, r); } break;
            case Op_table_8_g: { g = NS(table_8_ )((const skcms_Curve*)*args++, g); } break;
            case Op_table_8_b: { b = NS(table_8_ )((const skcms_Curve*)*args++, b); } break;
            case Op_table_8_a: { a = NS(table_8_ )((const skcms_Curve*)*args++, a); } break;

            case Op_table_16_r:{ r = NS(table_16_)((const skcms_Curve*)*args++, r); } break;
            case Op_table_16_g:{ g = NS(table_16_)((const skcms_Curve*)*args++, g); } break;
            case Op_table_16_b:{ b = NS(table_16_)((const skcms_Curve*)*args++, b); } break;
            case Op_table_16_a:{ a = NS(table_16_)((const skcms_Curve*)*args++, a); } break;

            case Op_clut_3D_8:{
                const skcms_A2B* a2b = (const skcms_A2B*) *args++;
                NS(clut_3_8_)(a2b, CAST(I32,F0),CAST(I32,F1), &r,&g,&b,a);
            } break;

            case Op_clut_3D_16:{
                const skcms_A2B* a2b = (const skcms_A2B*) *args++;
                NS(clut_3_16_)(a2b, CAST(I32,F0),CAST(I32,F1), &r,&g,&b,a);
            } break;

            case Op_clut_4D_8:{
                const skcms_A2B* a2b = (const skcms_A2B*) *args++;
                NS(clut_4_8_)(a2b, CAST(I32,F0),CAST(I32,F1), &r,&g,&b,a);
                // 'a' was really a CMYK K, so our output is actually opaque.
                a = F1;
            } break;

            case Op_clut_4D_16:{
                const skcms_A2B* a2b = (const skcms_A2B*) *args++;
                NS(clut_4_16_)(a2b, CAST(I32,F0),CAST(I32,F1), &r,&g,&b,a);
                // 'a' was really a CMYK K, so our output is actually opaque.
                a = F1;
            } break;

    // Notice, from here on down the store_ ops all return, ending the loop.

            case Op_store_a8: {
                U8 alpha = CAST(U8, to_fixed(a * 255));
                small_memcpy(dst + i, &alpha, N);
            } return;

            case Op_store_g8: {
                // g should be holding luminance (Y) (r,g,b ~~~> X,Y,Z)
                U8 gray = CAST(U8, to_fixed(g * 255));
                small_memcpy(dst + i, &gray, N);
            } return;

            case Op_store_4444: {
                U16 abgr = CAST(U16, to_fixed(r * 15) << 12)
                         | CAST(U16, to_fixed(g * 15) <<  8)
                         | CAST(U16, to_fixed(b * 15) <<  4)
                         | CAST(U16, to_fixed(a * 15) <<  0);
                small_memcpy(dst + 2*i, &abgr, 2*N);
            } return;

            case Op_store_565: {
                U16 rgb = CAST(U16, to_fixed(r * 31) <<  0 )
                        | CAST(U16, to_fixed(g * 63) <<  5 )
                        | CAST(U16, to_fixed(b * 31) << 11 );
                small_memcpy(dst + 2*i, &rgb, 2*N);
            } return;

            case Op_store_888: {
                uint8_t* rgb = (uint8_t*)dst + 3*i;
            #if defined(USING_NEON)
                // Same deal as load_888 but in reverse... we'll store using uint8x8x3_t, but
                // get there via U16 to save some instructions converting to float.  And just
                // like load_888, we'd prefer to go via U32 but for ARMv7 support.
                U16 R = CAST(U16, to_fixed(r * 255)),
                    G = CAST(U16, to_fixed(g * 255)),
                    B = CAST(U16, to_fixed(b * 255));

                uint8x8x3_t v = {{ (uint8x8_t)R, (uint8x8_t)G, (uint8x8_t)B }};
                vst3_lane_u8(rgb+0, v, 0);
                vst3_lane_u8(rgb+3, v, 2);
                vst3_lane_u8(rgb+6, v, 4);
                vst3_lane_u8(rgb+9, v, 6);
            #else
                STORE_3(rgb+0, CAST(U8, to_fixed(r * 255)) );
                STORE_3(rgb+1, CAST(U8, to_fixed(g * 255)) );
                STORE_3(rgb+2, CAST(U8, to_fixed(b * 255)) );
            #endif
            } return;

            case Op_store_8888: {
                U32 rgba = CAST(U32, to_fixed(r * 255) <<  0)
                         | CAST(U32, to_fixed(g * 255) <<  8)
                         | CAST(U32, to_fixed(b * 255) << 16)
                         | CAST(U32, to_fixed(a * 255) << 24);
                small_memcpy(dst + 4*i, &rgba, 4*N);
            } return;

            case Op_store_1010102: {
                U32 rgba = CAST(U32, to_fixed(r * 1023) <<  0)
                         | CAST(U32, to_fixed(g * 1023) << 10)
                         | CAST(U32, to_fixed(b * 1023) << 20)
                         | CAST(U32, to_fixed(a *    3) << 30);
                small_memcpy(dst + 4*i, &rgba, 4*N);
            } return;

            case Op_store_161616: {
                uintptr_t ptr = (uintptr_t)(dst + 6*i);
                assert( (ptr & 1) == 0 );                // The dst pointer must be 2-byte aligned
                uint16_t* rgb = (uint16_t*)ptr;          // for this cast to uint16_t* to be safe.
            #if defined(USING_NEON)
                uint16x4x3_t v = {{
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(r * 65535))),
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(g * 65535))),
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(b * 65535))),
                }};
                vst3_u16(rgb, v);
            #else
                I32 R = to_fixed(r * 65535),
                    G = to_fixed(g * 65535),
                    B = to_fixed(b * 65535);
                STORE_3(rgb+0, CAST(U16, (R & 0x00ff) << 8 | (R & 0xff00) >> 8) );
                STORE_3(rgb+1, CAST(U16, (G & 0x00ff) << 8 | (G & 0xff00) >> 8) );
                STORE_3(rgb+2, CAST(U16, (B & 0x00ff) << 8 | (B & 0xff00) >> 8) );
            #endif

            } return;

            case Op_store_16161616: {
                uintptr_t ptr = (uintptr_t)(dst + 8*i);
                assert( (ptr & 1) == 0 );               // The dst pointer must be 2-byte aligned
                uint16_t* rgba = (uint16_t*)ptr;        // for this cast to uint16_t* to be safe.
            #if defined(USING_NEON)
                uint16x4x4_t v = {{
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(r * 65535))),
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(g * 65535))),
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(b * 65535))),
                    (uint16x4_t)swap_endian_16(CAST(U16, to_fixed(a * 65535))),
                }};
                vst4_u16(rgba, v);
            #else
                U64 px = CAST(U64, to_fixed(r * 65535)) <<  0
                       | CAST(U64, to_fixed(g * 65535)) << 16
                       | CAST(U64, to_fixed(b * 65535)) << 32
                       | CAST(U64, to_fixed(a * 65535)) << 48;
                swap_endian_16x4(&px);
                small_memcpy(rgba, &px, 8*N);
            #endif
            } return;

            case Op_store_hhh: {
                uintptr_t ptr = (uintptr_t)(dst + 6*i);
                assert( (ptr & 1) == 0 );                // The dst pointer must be 2-byte aligned
                uint16_t* rgb = (uint16_t*)ptr;          // for this cast to uint16_t* to be safe.

                U16 R = Half_from_F(r),
                    G = Half_from_F(g),
                    B = Half_from_F(b);
            #if defined(USING_NEON)
                uint16x4x3_t v = {{
                    (uint16x4_t)R,
                    (uint16x4_t)G,
                    (uint16x4_t)B,
                }};
                vst3_u16(rgb, v);
            #else
                STORE_3(rgb+0, R);
                STORE_3(rgb+1, G);
                STORE_3(rgb+2, B);
            #endif
            } return;

            case Op_store_hhhh: {
                uintptr_t ptr = (uintptr_t)(dst + 8*i);
                assert( (ptr & 1) == 0 );                // The dst pointer must be 2-byte aligned
                uint16_t* rgba = (uint16_t*)ptr;         // for this cast to uint16_t* to be safe.

                U16 R = Half_from_F(r),
                    G = Half_from_F(g),
                    B = Half_from_F(b),
                    A = Half_from_F(a);
            #if defined(USING_NEON)
                uint16x4x4_t v = {{
                    (uint16x4_t)R,
                    (uint16x4_t)G,
                    (uint16x4_t)B,
                    (uint16x4_t)A,
                }};
                vst4_u16(rgba, v);
            #else
                U64 px = CAST(U64, R) <<  0
                       | CAST(U64, G) << 16
                       | CAST(U64, B) << 32
                       | CAST(U64, A) << 48;
                small_memcpy(rgba, &px, 8*N);
            #endif

            } return;

            case Op_store_fff: {
                uintptr_t ptr = (uintptr_t)(dst + 12*i);
                assert( (ptr & 3) == 0 );                // The dst pointer must be 4-byte aligned
                float* rgb = (float*)ptr;                // for this cast to float* to be safe.
            #if defined(USING_NEON)
                float32x4x3_t v = {{
                    (float32x4_t)r,
                    (float32x4_t)g,
                    (float32x4_t)b,
                }};
                vst3q_f32(rgb, v);
            #else
                STORE_3(rgb+0, r);
                STORE_3(rgb+1, g);
                STORE_3(rgb+2, b);
            #endif
            } return;

            case Op_store_ffff: {
                uintptr_t ptr = (uintptr_t)(dst + 16*i);
                assert( (ptr & 3) == 0 );                // The dst pointer must be 4-byte aligned
                float* rgba = (float*)ptr;               // for this cast to float* to be safe.
            #if defined(USING_NEON)
                float32x4x4_t v = {{
                    (float32x4_t)r,
                    (float32x4_t)g,
                    (float32x4_t)b,
                    (float32x4_t)a,
                }};
                vst4q_f32(rgba, v);
            #else
                STORE_4(rgba+0, r);
                STORE_4(rgba+1, g);
                STORE_4(rgba+2, b);
                STORE_4(rgba+3, a);
            #endif
            } return;
        }
    }
}

ATTR
static void NS(run_program)(const Op* program, const void** arguments,
                           const char* src, char* dst, int n,
                           const size_t src_bpp, const size_t dst_bpp) {
    int i = 0;
    while (n >= N) {
        NS(exec_ops)(program, arguments, src, dst, i);
        i += N;
        n -= N;
    }
    if (n > 0) {
        char tmp_src[4*4*N] = {0},
             tmp_dst[4*4*N] = {0};

        memcpy(tmp_src, (const char*)src + (size_t)i*src_bpp, (size_t)n*src_bpp);
        NS(exec_ops)(program, arguments, tmp_src, tmp_dst, 0);
        memcpy((char*)dst + (size_t)i*dst_bpp, tmp_dst, (size_t)n*dst_bpp);
    }
}

#if defined(USING_NEON) && defined(__clang__)
    #pragma clang diagnostic pop
#endif

// Clean up any #defines we may have set so that we can be #included again.

#if defined(USING_NEON)
    #undef  USING_NEON
#endif

#if defined(USING_NEON_F16C)
    #undef  USING_NEON_F16C
#endif

#if defined(USING_AVX_F16C)
    #undef  USING_AVX_F16C
#endif

#undef CAST

#if defined(LOAD_3)
    #undef  LOAD_3
#endif
#if defined(LOAD_4)
    #undef  LOAD_4
#endif
#if defined(STORE_3)
    #undef  STORE_3
#endif
#if defined(STORE_4)
    #undef  STORE_4
#endif
