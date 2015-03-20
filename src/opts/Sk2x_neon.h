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
    #include <arm_neon.h>
    #include <math.h>
    template <typename T> struct SkScalarToSIMD;
    template <> struct SkScalarToSIMD< float> { typedef float32x2_t Type; };
    #if defined(SK_CPU_ARM64)
        template <> struct SkScalarToSIMD<double> { typedef float64x2_t Type; };
    #else
        template <> struct SkScalarToSIMD<double> { typedef double Type[2];   };
    #endif


#elif defined(SK2X_PRIVATE)
    typename SkScalarToSIMD<T>::Type fVec;
    /*implicit*/ Sk2x(const typename SkScalarToSIMD<T>::Type vec) { fVec = vec; }

#else

#define M(...) template <> inline __VA_ARGS__ Sk2x<float>::

M() Sk2x() {}
M() Sk2x(float val)        { fVec = vdup_n_f32(val);    }
M() Sk2x(float a, float b) { fVec = (float32x2_t) { a, b }; }
M(Sk2f&) operator=(const Sk2f& o) { fVec = o.fVec; return *this; }

M(Sk2f) Load(const float vals[2]) { return vld1_f32(vals); }
M(void) store(float vals[2]) const { vst1_f32(vals, fVec); }

M(Sk2f)      add(const Sk2f& o) const { return vadd_f32(fVec, o.fVec); }
M(Sk2f) subtract(const Sk2f& o) const { return vsub_f32(fVec, o.fVec); }
M(Sk2f) multiply(const Sk2f& o) const { return vmul_f32(fVec, o.fVec); }

M(Sk2f) Min(const Sk2f& a, const Sk2f& b) { return vmin_f32(a.fVec, b.fVec); }
M(Sk2f) Max(const Sk2f& a, const Sk2f& b) { return vmax_f32(a.fVec, b.fVec); }

M(Sk2f) rsqrt() const {
    float32x2_t est0 = vrsqrte_f32(fVec),
                est1 = vmul_f32(vrsqrts_f32(fVec, vmul_f32(est0, est0)), est0);
    return est1;
}
M(Sk2f)  sqrt() const {
    float32x2_t est1 = this->rsqrt().fVec,
    // An extra step of Newton's method to refine the estimate of 1/sqrt(this).
                est2 = vmul_f32(vrsqrts_f32(fVec, vmul_f32(est1, est1)), est1);
    return vmul_f32(fVec, est2);
}

#undef M

#define M(...) template <> inline __VA_ARGS__ Sk2x<double>::

#if defined(SK_CPU_ARM64)
    M() Sk2x() {}
    M() Sk2x(double val)         { fVec = vdupq_n_f64(val);    }
    M() Sk2x(double a, double b) { fVec = (float64x2_t) { a, b }; }
    M(Sk2d&) operator=(const Sk2d& o) { fVec = o.fVec; return *this; }

    M(Sk2d) Load(const double vals[2]) { return vld1q_f64(vals); }
    M(void) store(double vals[2]) const { vst1q_f64(vals, fVec); }

    M(Sk2d)      add(const Sk2d& o) const { return vaddq_f64(fVec, o.fVec); }
    M(Sk2d) subtract(const Sk2d& o) const { return vsubq_f64(fVec, o.fVec); }
    M(Sk2d) multiply(const Sk2d& o) const { return vmulq_f64(fVec, o.fVec); }

    M(Sk2d) Min(const Sk2d& a, const Sk2d& b) { return vminq_f64(a.fVec, b.fVec); }
    M(Sk2d) Max(const Sk2d& a, const Sk2d& b) { return vmaxq_f64(a.fVec, b.fVec); }

    M(Sk2d) rsqrt() const {
        float64x2_t est0 = vrsqrteq_f64(fVec),
                    est1 = vmulq_f64(vrsqrtsq_f64(fVec, vmulq_f64(est0, est0)), est0);
        return est1;
    }
    M(Sk2d)  sqrt() const {
        float64x2_t est1 = this->rsqrt().fVec,
        // Two extra steps of Newton's method to refine the estimate of 1/sqrt(this).
                    est2 = vmulq_f64(vrsqrtsq_f64(fVec, vmulq_f64(est1, est1)), est1),
                    est3 = vmulq_f64(vrsqrtsq_f64(fVec, vmulq_f64(est2, est2)), est2);
        return vmulq_f64(fVec, est3);
    }

#else  // Scalar implementation for 32-bit chips, which don't have float64x2_t.
    M() Sk2x() {}
    M() Sk2x(double val)         { fVec[0] = fVec[1] = val; }
    M() Sk2x(double a, double b) { fVec[0] = a; fVec[1] = b; }
    M(Sk2d&) operator=(const Sk2d& o) {
        fVec[0] = o.fVec[0];
        fVec[1] = o.fVec[1];
        return *this;
    }

    M(Sk2d) Load(const double vals[2]) { return Sk2d(vals[0], vals[1]); }
    M(void) store(double vals[2]) const { vals[0] = fVec[0]; vals[1] = fVec[1]; }

    M(Sk2d)      add(const Sk2d& o) const { return Sk2d(fVec[0] + o.fVec[0], fVec[1] + o.fVec[1]); }
    M(Sk2d) subtract(const Sk2d& o) const { return Sk2d(fVec[0] - o.fVec[0], fVec[1] - o.fVec[1]); }
    M(Sk2d) multiply(const Sk2d& o) const { return Sk2d(fVec[0] * o.fVec[0], fVec[1] * o.fVec[1]); }

    M(Sk2d) Min(const Sk2d& a, const Sk2d& b) {
        return Sk2d(SkTMin(a.fVec[0], b.fVec[0]), SkTMin(a.fVec[1], b.fVec[1]));
    }
    M(Sk2d) Max(const Sk2d& a, const Sk2d& b) {
        return Sk2d(SkTMax(a.fVec[0], b.fVec[0]), SkTMax(a.fVec[1], b.fVec[1]));
    }

    M(Sk2d) rsqrt() const { return Sk2d(1.0/::sqrt(fVec[0]), 1.0/::sqrt(fVec[1])); }
    M(Sk2d)  sqrt() const { return Sk2d(    ::sqrt(fVec[0]),     ::sqrt(fVec[1])); }
#endif

#undef M

#endif
