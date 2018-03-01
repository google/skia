/*
 ============================================================================
 Name        : loongson-mmintrin.h
 Author      : Heiher <r@hev.cc>
 Version     : 0.0.1
 Copyright   : Copyright (c) 2017 everyone.
 Description : The helpers for x86 SSE to Loongson MMI.
 ============================================================================
 */

#ifndef __LOONGSON_MMINTRIN_H__
#define __LOONGSON_MMINTRIN_H__

#include <stdint.h>

typedef long long __m64i     __attribute__ ((__vector_size__ (8),  __may_alias__));
typedef long long __m128i    __attribute__ ((__vector_size__ (16), __may_alias__));

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_set1_epi8(int8_t __A)
{
    __m128i v;
    double z;

    __asm__ volatile (
        "ins    %[v], %[v], 8, 8 \n\t"
        "xor    %[z], %[z], %[z] \n\t"
        "mtc1   %[v], %[l]       \n\t"
        "pshufh %[l], %[l], %[z] \n\t"
        "mov.d  %[h], %[l]       \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [z]"=&f"(z), [v]"+&r"(__A)
    );

    return v;
}
static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_set1_epi16(int16_t __A)
{
    __m128i v;
    double z;

    __asm__ volatile (
        "xor    %[z], %[z], %[z] \n\t"
        "mtc1   %[v], %[l]       \n\t"
        "pshufh %[l], %[l], %[z] \n\t"
        "mov.d  %[h], %[l]       \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [z]"=&f"(z)
        : [v]"r"(__A)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_set1_epi32(int32_t __A)
{
    __m128i v;

    __asm__ volatile (
        "mtc1      %[v], %[l]       \n\t"
        "punpcklwd %[l], %[l], %[l] \n\t"
        "mov.d     %[h], %[l]       \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [v]"r"(__A)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_setr_epi8(int8_t __A, int8_t __B, int8_t __C, int8_t __D, int8_t __E, int8_t __F, int8_t __G,
              int8_t __H, int8_t __I, int8_t __J, int8_t __K, int8_t __L, int8_t __M, int8_t __N,
              int8_t __O, int8_t __P)
{
    __m128i v;

    __asm__ volatile (
        "dins  %[a], %[b], 8,  8 \n\t"
        "dins  %[a], %[c], 16, 8 \n\t"
        "dins  %[a], %[d], 24, 8 \n\t"
        "dins  %[a], %[e], 32, 8 \n\t"
        "dins  %[a], %[f], 40, 8 \n\t"
        "dins  %[a], %[g], 48, 8 \n\t"
        "dins  %[a], %[h], 56, 8 \n\t"
        "dins  %[i], %[j], 8, 8  \n\t"
        "dins  %[i], %[k], 16, 8 \n\t"
        "dins  %[i], %[l], 24, 8 \n\t"
        "dins  %[i], %[m], 32, 8 \n\t"
        "dins  %[i], %[n], 40, 8 \n\t"
        "dins  %[i], %[o], 48, 8 \n\t"
        "dins  %[i], %[p], 56, 8 \n\t"
        "dmtc1 %[a], %[vl]       \n\t"
        "dmtc1 %[i], %[vh]       \n\t"
        : [vl]"=&f"(v[0]), [vh]"=&f"(v[1]), [a]"+&r"(__A), [i]"+&r"(__I)
        : [b]"r"(__B), [c]"r"(__C), [d]"r"(__D), [e]"r"(__E), [f]"r"(__F), [g]"r"(__G),
          [h]"r"(__H), [j]"r"(__J), [k]"r"(__K), [l]"r"(__L), [m]"r"(__M), [n]"r"(__N),
          [o]"r"(__O), [p]"r"(__P)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_set_epi8(int8_t __A, int8_t __B, int8_t __C, int8_t __D, int8_t __E, int8_t __F, int8_t __G,
             int8_t __H, int8_t __I, int8_t __J, int8_t __K, int8_t __L, int8_t __M, int8_t __N,
             int8_t __O, int8_t __P)
{
    return _mm_setr_epi8(__P, __O, __N, __M, __L, __K, __J, __I,
                         __H, __G, __F, __E, __D, __C, __B, __A);
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_setr_epi16(int16_t __A, int16_t __B, int16_t __C, int16_t __D, int16_t __E, int16_t __F,
               int16_t __G, int16_t __H)
{
    __m128i v;

    __asm__ volatile (
            "dins  %[a], %[b], 16, 16 \n\t"
            "dins  %[a], %[c], 32, 16 \n\t"
            "dins  %[a], %[d], 48, 16 \n\t"
            "dins  %[e], %[f], 16, 16 \n\t"
            "dins  %[e], %[g], 32, 16 \n\t"
            "dins  %[e], %[h], 48, 16 \n\t"
            "dmtc1 %[a], %[vl]        \n\t"
            "dmtc1 %[e], %[vh]        \n\t"
            : [vl]"=&f"(v[0]), [vh]"=&f"(v[1]), [a]"+&r"(__A), [e]"+&r"(__E)
            : [b]"r"(__B), [c]"r"(__C), [d]"r"(__D), [f]"r"(__F), [g]"r"(__G), [h]"r"(__H)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_set_epi16(int16_t __A, int16_t __B, int16_t __C, int16_t __D,
              int16_t __E, int16_t __F, int16_t __G, int16_t __H)
{
    return _mm_setr_epi16(__H, __G, __F, __E, __D, __C, __B, __A);
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_setr_epi32(int32_t __A, int32_t __B, int32_t __C, int32_t __D)
{
    __m128i v;

    __asm__ volatile (
        "mtc1  %[a], %[l] \n\t"
        "mthc1 %[b], %[l] \n\t"
        "mtc1  %[c], %[h] \n\t"
        "mthc1 %[d], %[h] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [a]"r"(__A), [b]"r"(__B), [c]"r"(__C), [d]"r"(__D)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_set_epi32(int32_t __A, int32_t __B, int32_t __C, int32_t __D)
{
    return _mm_setr_epi32(__D, __C, __B, __A);
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_loadu_cvtsi16_si128(int16_t const *__P)
{
    __m128i v;
    int32_t x;

    __asm__ volatile (
        "ulhu %[x], %[p]       \n\t"
        "mtc1 %[x], %[l]       \n\t"
        "xor  %[h], %[h], %[h] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [x]"=&r"(x)
        : [p]"m"(*__P)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_loadu_cvtsi32_si128(int32_t const *__P)
{
    __m128i v;

    __asm__ volatile (
        "gslwlc1 %[l], 0x3(%[p])  \n\t"
        "gslwrc1 %[l], 0x0(%[p])  \n\t"
        "xor     %[h], %[h], %[h] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [p]"r"(__P)
        : "memory"
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cvtsi32_si128(int32_t __A)
{
    __m128i v;

    __asm__ volatile (
        "mtc1 %[a], %[l]       \n\t"
        "xor  %[h], %[h], %[h] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [a]"r"(__A)
    );

    return v;
}

static inline int32_t __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cvtsi128_si32(__m128i __A)
{
    int32_t v;

    __asm__ volatile (
        "mfc1 %[v], %[l] \n\t"
        : [v]"=&r"(v)
        : [l]"f"(__A[0])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_setzero_si128(void)
{
    __m128i v;

    __asm__ volatile (
        "xor %[l], %[l], %[l] \n\t"
        "xor %[h], %[h], %[h] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_loadl_epi64(__m128i const *__P)
{
    __m128i v;

    __asm__ volatile (
        "gsldlc1 %[l], 0x7(%[p])  \n\t"
        "gsldrc1 %[l], 0x0(%[p])  \n\t"
        "xor     %[h], %[h], %[h] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [p]"r"(__P)
        : "memory"
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_load_si128(__m128i const *__P)
{
    __m128i v;

    __asm__ volatile (
        "gslqc1 %[h], %[l], (%[p]) \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [p]"r"(__P)
        : "memory"
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_loadu_si128(__m128i const *__P)
{
    __m128i v;

    __asm__ volatile (
        "gsldlc1 %[l], 0x7(%[p]) \n\t"
        "gsldrc1 %[l], 0x0(%[p]) \n\t"
        "gsldlc1 %[h], 0xf(%[p]) \n\t"
        "gsldrc1 %[h], 0x8(%[p]) \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [p]"r"(__P)
        : "memory"
    );

    return v;
}

static inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_storel_epi64(__m128i *__P, __m128i __A)
{
    __asm__ volatile (
        "gssdlc1 %[l], 0x7(%[p]) \n\t"
        "gssdrc1 %[l], 0x0(%[p]) \n\t"
        :: [l]"f"(__A[0]), [p]"r"(__P)
        : "memory"
    );
}

static inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_store_si128(__m128i *__P, __m128i __A)
{
    __asm__ volatile (
        "gssqc1 %[h], %[l], (%[p]) \n\t"
        :: [l]"f"(__A[0]), [h]"f"(__A[1]), [p]"r"(__P)
        : "memory"
    );
}

static inline void __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_storeu_si128(__m128i* __P, __m128i __A)
{
    __asm__ volatile (
        "gssqc1 %[h], %[l], (%[p]) \n\t"
        :: [l]"f"(__A[0]), [h]"f"(__A[1]), [p]"r"(__P)
        : "memory"
    );
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_add_epi8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "paddb %[l], %[al], %[bl] \n\t"
        "paddb %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_adds_epu8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "paddusb %[l], %[al], %[bl] \n\t"
        "paddusb %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_add_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "paddh %[l], %[al], %[bl] \n\t"
        "paddh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_add_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "paddw %[l], %[al], %[bl] \n\t"
        "paddw %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sub_epi8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "psubb %[l], %[al], %[bl] \n\t"
        "psubb %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sub_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "psubh %[l], %[al], %[bl] \n\t"
        "psubh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_sub_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "psubw %[l], %[al], %[bl] \n\t"
        "psubw %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_mullo_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pmullh %[l], %[al], %[bl] \n\t"
        "pmullh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_mulhi_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pmulhh %[l], %[al], %[bl] \n\t"
        "pmulhh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_mulhi_epu16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pmulhuh %[l], %[al], %[bl] \n\t"
        "pmulhuh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_mul_epu32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pmuluw %[l], %[al], %[bl] \n\t"
        "pmuluw %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_and_si128(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "and %[l], %[al], %[bl] \n\t"
        "and %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_andnot_si128(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pandn %[l], %[al], %[bl] \n\t"
        "pandn %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_or_si128(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "or %[l], %[al], %[bl] \n\t"
        "or %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_xor_si128(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "xor %[l], %[al], %[bl] \n\t"
        "xor %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_slli_epi16(__m128i __A, int32_t __S)
{
    __m128i v;

    __asm__ volatile (
        "psllh %[l], %[al], %[s] \n\t"
        "psllh %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__S)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_slli_epi32(__m128i __A, int32_t __S)
{
    __m128i v;

    __asm__ volatile (
        "psllw %[l], %[al], %[s] \n\t"
        "psllw %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__S)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_srli_epi16(__m128i __A, int32_t __S)
{
    __m128i v;

    __asm__ volatile (
        "psrlh %[l], %[al], %[s] \n\t"
        "psrlh %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__S)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_srli_epi32(__m128i __A, int32_t __S)
{
    __m128i v;

    __asm__ volatile (
        "psrlw %[l], %[al], %[s] \n\t"
        "psrlw %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__S)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_srli_si128(__m128i __A, int32_t __S)
{
    __m128i v;
    uint64_t s, t;

    __S = __S > 15 ? 16 : __S;

    if (__S > 8) {
        s = (__S - 8) << 3;

        __asm__ volatile (
            "xor  %[h], %[h],  %[h] \n\t"
            "dsrl %[l], %[ah], %[s] \n\t"
            : [l]"=&f"(v[0]), [h]"=&f"(v[1])
            : [ah]"f"(__A[1]), [s]"f"(s)
        );
    } else {
        s = __S << 3;
        t = 64 - s;

        __asm__ volatile (
            "dsll %[t], %[ah], %[t] \n\t"
            "dsrl %[h], %[ah], %[s] \n\t"
            "dsrl %[l], %[al], %[s] \n\t"
            "or   %[l], %[l],  %[t] \n\t"
            : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [t]"+&f"(t)
            : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(s)
        );
    }

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_srai_epi16(__m128i __A, int32_t __S)
{
    __m128i v;

    __asm__ volatile (
        "psrah %[l], %[al], %[s] \n\t"
        "psrah %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__S)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_srai_epi32(__m128i __A, int32_t __S)
{
    __m128i v;

    __asm__ volatile (
        "psraw %[l], %[al], %[s] \n\t"
        "psraw %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__S)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shufflelo_epi16(__m128i __A, int32_t __mask)
{
    __m128i v;

    __asm__ volatile (
        "pshufh %[l], %[al], %[s] \n\t"
        "mov.d  %[h], %[ah]       \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__mask)
    );

    return v;
}

#define _MM_SHUFFLE(z, y, x, w) ((z << 6) | (y << 4) | (x << 2) | w)

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shufflehi_epi16(__m128i __A, int32_t __mask)
{
    __m128i v;

    __asm__ volatile (
        "mov.d  %[l], %[al]       \n\t"
        "pshufh %[h], %[ah], %[s] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [s]"f"(__mask)
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shuffle_epi32_0000(__m128i __A)
{
    __m128i v;
    uint64_t t;

    __asm__ volatile (
        "mfc1  %[t], %[al] \n\t"
        "mov.d %[l], %[al] \n\t"
        "mthc1 %[t], %[l]  \n\t"
        "mov.d %[h], %[l]  \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [t]"=&r"(t)
        : [al]"f"(__A[0]), [ah]"f"(__A[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shuffle_epi32_0020(__m128i __A)
{
    __m128i v;
    uint64_t t;

    __asm__ volatile (
        "mfc1  %[t], %[ah] \n\t"
        "mov.d %[l], %[al] \n\t"
        "mov.d %[h], %[al] \n\t"
        "mthc1 %[t], %[l]  \n\t"
        "mfc1  %[t], %[al] \n\t"
        "mthc1 %[t], %[h]  \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [t]"=&r"(t)
        : [al]"f"(__A[0]), [ah]"f"(__A[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_shuffle_epi32_3232(__m128i __A)
{
    __m128i v;

    __asm__ volatile (
        "mov.d %[l], %[ah] \n\t"
        "mov.d %[h], %[ah] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_min_epu8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pminub %[l], %[al], %[bl] \n\t"
        "pminub %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_min_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pminsh %[l], %[al], %[bl] \n\t"
        "pminsh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_max_epu8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pmaxub %[l], %[al], %[bl] \n\t"
        "pmaxub %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_max_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pmaxsh %[l], %[al], %[bl] \n\t"
        "pmaxsh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cmpeq_epi8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pcmpeqb %[l], %[al], %[bl] \n\t"
        "pcmpeqb %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cmpeq_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pcmpeqh %[l], %[al], %[bl] \n\t"
        "pcmpeqh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cmpeq_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pcmpeqw %[l], %[al], %[bl] \n\t"
        "pcmpeqw %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cmplt_epi8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pcmpgtb %[l], %[bl], %[al] \n\t"
        "pcmpgtb %[h], %[bh], %[ah] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cmplt_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pcmpgtw %[l], %[bl], %[al] \n\t"
        "pcmpgtw %[h], %[bh], %[ah] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_cmpgt_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "pcmpgtw %[l], %[al], %[bl] \n\t"
        "pcmpgtw %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_packus_epi16(__m128i __A, __m128i __B)
{
    __m128i v;
    double t;

    __asm__ volatile (
        "packushb  %[t], %[ah], %[bh] \n\t"
        "packushb  %[l], %[al], %[bl] \n\t"
        "punpckhwd %[h], %[l],  %[t]  \n\t"
        "punpcklwd %[l], %[l],  %[t]  \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [t]"=&f"(t)
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_packs_epi32(__m128i __A, __m128i __B)
{
    __m128i v;
    double t;

    __asm__ volatile (
        "packsswh  %[t], %[ah], %[bh] \n\t"
        "packsswh  %[l], %[al], %[bl] \n\t"
        "punpckhwd %[h], %[l],  %[t]  \n\t"
        "punpcklwd %[l], %[l],  %[t]  \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1]), [t]"=&f"(t)
        : [al]"f"(__A[0]), [ah]"f"(__A[1]), [bl]"f"(__B[0]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpacklo_epi8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "punpckhbh %[h], %[al], %[bl] \n\t"
        "punpcklbh %[l], %[al], %[bl] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [bl]"f"(__B[0])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpacklo_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "punpckhhw %[h], %[al], %[bl] \n\t"
        "punpcklhw %[l], %[al], %[bl] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [bl]"f"(__B[0])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpacklo_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "punpckhwd %[h], %[al], %[bl] \n\t"
        "punpcklwd %[l], %[al], %[bl] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [bl]"f"(__B[0])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpacklo_epi64(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "mov.d %[l], %[al] \n\t"
        "mov.d %[h], %[bl] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [al]"f"(__A[0]), [bl]"f"(__B[0])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpackhi_epi8(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "punpcklbh %[l], %[ah], %[bh] \n\t"
        "punpckhbh %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [ah]"f"(__A[1]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpackhi_epi16(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "punpcklhw %[l], %[ah], %[bh] \n\t"
        "punpckhhw %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [ah]"f"(__A[1]), [bh]"f"(__B[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpackhi_epi32(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "punpcklwd %[l], %[ah], %[bh] \n\t"
        "punpckhwd %[h], %[ah], %[bh] \n\t"
        : [l]"=&f"(v[0]), [h]"=&f"(v[1])
        : [ah]"f"(__A[1]), [bh]"f"(__B[1])
    );

    return v;
}

static inline int32_t __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_movemask_epi8(__m128i __A)
{
    int32_t v;
    __m64i h;
    __m64i l;

    __asm__ volatile (
        "pmovmskb  %[l], %[al]      \n\t"
        "pmovmskb  %[h], %[ah]      \n\t"
        "punpcklbh %[l], %[l], %[h] \n\t"
        "mfc1      %[v], %[l]       \n\t"
        : [v]"=&r"(v), [l]"=&f"(l), [h]"=&f"(h)
        : [al]"f"(__A[0]), [ah]"f"(__A[1])
    );

    return v;
}

static inline __m128i __attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_unpackhi_epi64(__m128i __A, __m128i __B)
{
    __m128i v;

    __asm__ volatile (
        "mov.d  %[vl], %[ah] \n\t"
        "mov.d  %[vh], %[bh] \n\t"
        : [vh]"=&f"(v[1]), [vl]"=&f"(v[0])
        : [ah]"f"(__A[1]), [bh]"f"(__B[1])
    );

    return v;
}

#endif /* __LOONGSON_MMINTRIN_H__ */
