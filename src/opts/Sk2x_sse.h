/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

#include "SkTypes.h"  // Keep this before any #ifdef for skbug.com/3362

#if defined(SK2X_PREAMBLE)
    #include <immintrin.h>
    template <typename T> struct SkScalarToSIMD;
    template <> struct SkScalarToSIMD< float> { typedef __m128  Type; };
    template <> struct SkScalarToSIMD<double> { typedef __m128d Type; };


#elif defined(SK2X_PRIVATE)
    typename SkScalarToSIMD<T>::Type fVec;
    /*implicit*/ Sk2x(const typename SkScalarToSIMD<T>::Type vec) { fVec = vec; }

#else

#define M(...) template <> inline __VA_ARGS__ Sk2x<float>::

M() Sk2x() {}
M() Sk2x(float val)        { fVec = _mm_set1_ps(val);    }
M() Sk2x(float a, float b) { fVec = _mm_set_ps(b,a,b,a); }
M(Sk2f&) operator=(const Sk2f& o) { fVec = o.fVec; return *this; }

M(Sk2f) Load(const float vals[2]) {
    return _mm_castsi128_ps(_mm_loadl_epi64((const __m128i*)vals));
}
M(void) store(float vals[2]) const { _mm_storel_pi((__m64*)vals, fVec); }

M(Sk2f)      add(const Sk2f& o) const { return _mm_add_ps(fVec, o.fVec); }
M(Sk2f) subtract(const Sk2f& o) const { return _mm_sub_ps(fVec, o.fVec); }
M(Sk2f) multiply(const Sk2f& o) const { return _mm_mul_ps(fVec, o.fVec); }
M(Sk2f)   divide(const Sk2f& o) const { return _mm_div_ps(fVec, o.fVec); }

M(Sk2f) Min(const Sk2f& a, const Sk2f& b) { return _mm_min_ps(a.fVec, b.fVec); }
M(Sk2f) Max(const Sk2f& a, const Sk2f& b) { return _mm_max_ps(a.fVec, b.fVec); }

M(Sk2f) rsqrt() const { return _mm_rsqrt_ps(fVec); }
M(Sk2f)  sqrt() const { return _mm_sqrt_ps (fVec); }

M(Sk2f)       invert() const { return Sk2f(1.0f) / *this; }
M(Sk2f) approxInvert() const { return _mm_rcp_ps(fVec); }

#undef M

#define M(...) template <> inline __VA_ARGS__ Sk2x<double>::

M() Sk2x() {}
M() Sk2x(double val)         { fVec = _mm_set1_pd(val);    }
M() Sk2x(double a, double b) { fVec = _mm_set_pd(b, a);    }
M(Sk2d&) operator=(const Sk2d& o) { fVec = o.fVec; return *this; }

M(Sk2d) Load(const double vals[2]) { return _mm_loadu_pd(vals); }
M(void) store(double vals[2]) const { _mm_storeu_pd(vals, fVec); }

M(Sk2d)      add(const Sk2d& o) const { return _mm_add_pd(fVec, o.fVec); }
M(Sk2d) subtract(const Sk2d& o) const { return _mm_sub_pd(fVec, o.fVec); }
M(Sk2d) multiply(const Sk2d& o) const { return _mm_mul_pd(fVec, o.fVec); }
M(Sk2d)   divide(const Sk2d& o) const { return _mm_div_pd(fVec, o.fVec); }

M(Sk2d) Min(const Sk2d& a, const Sk2d& b) { return _mm_min_pd(a.fVec, b.fVec); }
M(Sk2d) Max(const Sk2d& a, const Sk2d& b) { return _mm_max_pd(a.fVec, b.fVec); }

// There is no _mm_rsqrt_pd, so we do Sk2d::rsqrt() in floats.
M(Sk2d) rsqrt() const { return _mm_cvtps_pd(_mm_rsqrt_ps(_mm_cvtpd_ps(fVec))); }
M(Sk2d)  sqrt() const { return _mm_sqrt_pd(fVec); }

// No _mm_rcp_pd, so do Sk2d::approxInvert() in floats.
M(Sk2d)       invert() const { return Sk2d(1.0) / *this; }
M(Sk2d) approxInvert() const { return _mm_cvtps_pd(_mm_rcp_ps(_mm_cvtpd_ps(fVec))); }

#undef M

#endif
