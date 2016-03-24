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

    void transpose(Sk4f* x, Sk4f* y, Sk4f* z, Sk4f* w) const {
        auto t = Transpose(r,g,b,a);
        *x = t.r;
        *y = t.g;
        *z = t.b;
        *w = t.a;
    }
    void transpose(  float[16]) const;
    void transpose(uint8_t[16]) const;
};

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

#elif defined(SK_ARM_HAS_NEON)

inline Sk4x4f Sk4x4f::Transpose(const Sk4f& x, const Sk4f& y, const Sk4f& z, const Sk4f& w) {
    float32x4x2_t xy = vuzpq_f32(x.fVec, y.fVec),
                  zw = vuzpq_f32(z.fVec, w.fVec),
                  rb = vuzpq_f32(xy.val[0], zw.val[0]),
                  ga = vuzpq_f32(xy.val[1], zw.val[1]);
    return { rb.val[0], ga.val[0], rb.val[1], ga.val[1] };
}

inline Sk4x4f Sk4x4f::Transpose(const float fs[16]) {
    float32x4x4_t v = vld4q_f32(fs);
    return { v.val[0], v.val[1], v.val[2], v.val[3] };
}

inline Sk4x4f Sk4x4f::Transpose(const uint8_t bs[16]) {
    auto b16 = vreinterpretq_u32_u8(vld1q_u8(bs));
    auto r =   vcvtq_f32_u32(vandq_u32(vdupq_n_u32(0x000000FF), b16)    ),
         g = vcvtq_n_f32_u32(vandq_u32(vdupq_n_u32(0x0000FF00), b16),  8),
         b = vcvtq_n_f32_u32(vandq_u32(vdupq_n_u32(0x00FF0000), b16), 16),
         a = vcvtq_n_f32_u32(vandq_u32(vdupq_n_u32(0xFF000000), b16), 24);
    return { r,g,b,a };
}

inline void Sk4x4f::transpose(float fs[16]) const {
    float32x4x4_t v = {{ r.fVec, g.fVec, b.fVec, a.fVec }};
    vst4q_f32(fs, v);
}

inline void Sk4x4f::transpose(uint8_t bs[16]) const {
    auto R = vandq_u32(vdupq_n_u32(0x000000FF),   vcvtq_u32_f32(r.fVec    )),
         G = vandq_u32(vdupq_n_u32(0x0000FF00), vcvtq_n_u32_f32(g.fVec,  8)),
         B = vandq_u32(vdupq_n_u32(0x00FF0000), vcvtq_n_u32_f32(b.fVec, 16)),
         A = vandq_u32(vdupq_n_u32(0xFF000000), vcvtq_n_u32_f32(a.fVec, 24));
    vst1q_u8(bs, vreinterpretq_u8_u32(vorrq_u32(A, vorrq_u32(B, vorrq_u32(G, R)))));
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
