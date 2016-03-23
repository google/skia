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

// TODO: NEON

#if 1 && !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2

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

    auto mask = _mm_set1_epi32(0xFF);
    auto r = _mm_cvtepi32_ps(_mm_and_si128(mask,               (b16    ))),
         g = _mm_cvtepi32_ps(_mm_and_si128(mask, _mm_srli_epi32(b16,  8))),
         b = _mm_cvtepi32_ps(_mm_and_si128(mask, _mm_srli_epi32(b16, 16))),
         a = _mm_cvtepi32_ps(                    _mm_srli_epi32(b16, 24));
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
    auto R =                _mm_cvttps_epi32(r.fVec),
         G = _mm_slli_epi32(_mm_cvttps_epi32(g.fVec),  8),
         B = _mm_slli_epi32(_mm_cvttps_epi32(b.fVec), 16),
         A = _mm_slli_epi32(_mm_cvttps_epi32(a.fVec), 24);
    _mm_storeu_si128((__m128i*)bs, _mm_or_si128(A, _mm_or_si128(B, _mm_or_si128(G, R))));
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
