/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4x4f_DEFINED
#define Sk4x4f_DEFINED

#include "SkNx.h"

struct Sk4x4f {
    Sk4f r,g,b,a;

    static Sk4x4f Transpose(const Sk4f&, const Sk4f&, const Sk4f&, const Sk4f&);
    static Sk4x4f Transpose(const   float[16]);
    static Sk4x4f Transpose(const uint8_t[16]);

    void transpose(Sk4f*, Sk4f*, Sk4f*, Sk4f*) const;
    void transpose(  float[16]) const;
    void transpose(uint8_t[16]) const;
};

// TODO: SSE2, NEON

#if 1 && !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3

inline Sk4x4f Sk4x4f::Transpose(const Sk4f& x, const Sk4f& y, const Sk4f& z, const Sk4f& w) {
    auto r = x.fVec,
         g = y.fVec,
         b = z.fVec,
         a = w.fVec;
    _MM_TRANSPOSE4_PS(r,g,b,a);
    return { r,g,b,a };
}

inline Sk4x4f Sk4x4f::Transpose(const float fs[16]) {
    return Transpose(Sk4f::Load(fs+0), Sk4f::Load(fs+4), Sk4f::Load(fs+8), Sk4f::Load(fs+12));
}

inline Sk4x4f Sk4x4f::Transpose(const uint8_t bs[16]) {
    auto b16 = _mm_loadu_si128((const __m128i*)bs);
    auto _ = ~0;  // Shuffles in a zero byte.
    auto r = _mm_cvtepi32_ps(
            _mm_shuffle_epi8(b16, _mm_setr_epi8(0,_,_,_,4,_,_,_, 8,_,_,_,12,_,_,_)));
    auto g = _mm_cvtepi32_ps(
            _mm_shuffle_epi8(b16, _mm_setr_epi8(1,_,_,_,5,_,_,_, 9,_,_,_,13,_,_,_)));
    auto b = _mm_cvtepi32_ps(
            _mm_shuffle_epi8(b16, _mm_setr_epi8(2,_,_,_,6,_,_,_,10,_,_,_,14,_,_,_)));
    auto a = _mm_cvtepi32_ps(
            _mm_shuffle_epi8(b16, _mm_setr_epi8(3,_,_,_,7,_,_,_,11,_,_,_,15,_,_,_)));
    return { r,g,b,a };
}

inline void Sk4x4f::transpose(Sk4f* x, Sk4f* y, Sk4f* z, Sk4f* w) const {
    auto R = r.fVec,
         G = g.fVec,
         B = b.fVec,
         A = a.fVec;
    _MM_TRANSPOSE4_PS(R,G,B,A);
    *x = R;
    *y = G;
    *z = B;
    *w = A;
}

inline void Sk4x4f::transpose(float fs[16]) const {
    Sk4f x,y,z,w;
    this->transpose(&x,&y,&z,&w);
    x.store(fs+ 0);
    y.store(fs+ 4);
    z.store(fs+ 8);
    w.store(fs+12);
}

inline void Sk4x4f::transpose(uint8_t bs[16]) const {
    auto packed = _mm_packus_epi16(_mm_packus_epi16(_mm_cvttps_epi32(r.fVec),
                                                    _mm_cvttps_epi32(g.fVec)),
                                   _mm_packus_epi16(_mm_cvttps_epi32(b.fVec),
                                                    _mm_cvttps_epi32(a.fVec)));
    _mm_storeu_si128((__m128i*)bs, _mm_shuffle_epi8(packed, _mm_setr_epi8(0, 4,  8, 12,
                                                                          1, 5,  9, 13,
                                                                          2, 6, 10, 14,
                                                                          3, 7, 11, 15)));
}

#else

inline Sk4x4f Sk4x4f::Transpose(const Sk4f& x, const Sk4f& y, const Sk4f& z, const Sk4f& w) {
    return {
        { x[0], y[0], z[0], w[0] },
        { x[1], y[1], z[1], w[1] },
        { x[2], y[2], z[2], w[2] },
        { x[3], y[3], z[3], w[3] },
    };
}

inline Sk4x4f Sk4x4f::Transpose(const float fs[16]) {
    return Transpose(Sk4f::Load(fs+0), Sk4f::Load(fs+4), Sk4f::Load(fs+8), Sk4f::Load(fs+12));
}

inline Sk4x4f Sk4x4f::Transpose(const uint8_t bs[16]) {
    return {
        { (float)bs[0], (float)bs[4], (float)bs[ 8], (float)bs[12] },
        { (float)bs[1], (float)bs[5], (float)bs[ 9], (float)bs[13] },
        { (float)bs[2], (float)bs[6], (float)bs[10], (float)bs[14] },
        { (float)bs[3], (float)bs[7], (float)bs[11], (float)bs[15] },
    };
}

inline void Sk4x4f::transpose(Sk4f* x, Sk4f* y, Sk4f* z, Sk4f* w) const {
    *x = { r[0], g[0], b[0], a[0] };
    *y = { r[1], g[1], b[1], a[1] };
    *z = { r[2], g[2], b[2], a[2] };
    *w = { r[3], g[3], b[3], a[3] };
}

inline void Sk4x4f::transpose(float fs[16]) const {
    Sk4f x,y,z,w;
    this->transpose(&x,&y,&z,&w);
    x.store(fs+ 0);
    y.store(fs+ 4);
    z.store(fs+ 8);
    w.store(fs+12);
}

inline void Sk4x4f::transpose(uint8_t bs[16]) const {
    bs[ 0] = (uint8_t)r[0]; bs[ 1] = (uint8_t)g[0]; bs[ 2] = (uint8_t)b[0]; bs[ 3] = (uint8_t)a[0];
    bs[ 4] = (uint8_t)r[1]; bs[ 5] = (uint8_t)g[1]; bs[ 6] = (uint8_t)b[1]; bs[ 7] = (uint8_t)a[1];
    bs[ 8] = (uint8_t)r[2]; bs[ 9] = (uint8_t)g[2]; bs[10] = (uint8_t)b[2]; bs[11] = (uint8_t)a[2];
    bs[12] = (uint8_t)r[3]; bs[13] = (uint8_t)g[3]; bs[14] = (uint8_t)b[3]; bs[15] = (uint8_t)a[3];
}

#endif

#endif//Sk4x4f_DEFINED
