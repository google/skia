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
    #include "SkFloatingPoint.h"
    #include <math.h>

#elif defined(SK2X_PRIVATE)
    T fVec[2];

#else

#define M(...) template <typename T> __VA_ARGS__ Sk2x<T>::

M() Sk2x() {}
M() Sk2x(T val) { fVec[0] = fVec[1] = val; }
M() Sk2x(T a, T b) { fVec[0] = a; fVec[1] = b; }

M(Sk2x<T>&) operator=(const Sk2x<T>& o) {
    fVec[0] = o.fVec[0];
    fVec[1] = o.fVec[1];
    return *this;
}

M(Sk2x<T>) Load(const T vals[2]) { return Sk2x<T>(vals[0], vals[1]); }
M(void) store(T vals[2]) const { vals[0] = fVec[0]; vals[1] = fVec[1]; }

M(Sk2x<T>) add(const Sk2x<T>& o) const {
    return Sk2x<T>(fVec[0] + o.fVec[0], fVec[1] + o.fVec[1]);
}
M(Sk2x<T>) subtract(const Sk2x<T>& o) const {
    return Sk2x<T>(fVec[0] - o.fVec[0], fVec[1] - o.fVec[1]);
}
M(Sk2x<T>) multiply(const Sk2x<T>& o) const {
    return Sk2x<T>(fVec[0] * o.fVec[0], fVec[1] * o.fVec[1]);
}
M(Sk2x<T>) divide(const Sk2x<T>& o) const {
    return Sk2x<T>(fVec[0] / o.fVec[0], fVec[1] / o.fVec[1]);
}

M(Sk2x<T>) Min(const Sk2x<T>& a, const Sk2x<T>& b) {
    return Sk2x<T>(SkTMin(a.fVec[0], b.fVec[0]), SkTMin(a.fVec[1], b.fVec[1]));
}
M(Sk2x<T>) Max(const Sk2x<T>& a, const Sk2x<T>& b) {
    return Sk2x<T>(SkTMax(a.fVec[0], b.fVec[0]), SkTMax(a.fVec[1], b.fVec[1]));
}

#undef M

#define M template <> inline

M Sk2f Sk2f::rsqrt() const { return Sk2f(sk_float_rsqrt(fVec[0]), sk_float_rsqrt(fVec[1])); }
M Sk2f Sk2f:: sqrt() const { return Sk2f(         sqrtf(fVec[0]),          sqrtf(fVec[1])); }

M Sk2d Sk2d::rsqrt() const { return Sk2d(1.0/::sqrt(fVec[0]), 1.0/::sqrt(fVec[1])); }
M Sk2d Sk2d:: sqrt() const { return Sk2d(    ::sqrt(fVec[0]),     ::sqrt(fVec[1])); }

#undef M

#endif
