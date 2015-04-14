/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_DEFINED
#define SkNx_DEFINED


#define SKNX_NO_SIMDx  // Remove the x to disable SIMD for all SkNx types.


#include "SkScalar.h"
#include "SkTypes.h"
#include <math.h>
#define REQUIRE(x) static_assert(x, #x)

// The default implementations just fall back on a pair of size N/2.

// SkNb is a _very_ minimal class representing a vector of bools returned by comparison operators.
// We pass along the byte size of the compared types (Bytes) to help platform specializations.
template <int N, int Bytes>
class SkNb {
public:
    SkNb() {}
    SkNb(const SkNb<N/2, Bytes>& lo, const SkNb<N/2, Bytes>& hi) : fLo(lo), fHi(hi) {}

    bool allTrue() const { return fLo.allTrue() && fHi.allTrue(); }
    bool anyTrue() const { return fLo.anyTrue() || fHi.anyTrue(); }

private:
    REQUIRE(0 == (N & (N-1)));
    SkNb<N/2, Bytes> fLo, fHi;
};

template <int N, typename T>
class SkNf {
    typedef SkNb<N, sizeof(T)> Nb;
public:
    SkNf() {}
    explicit SkNf(T val) : fLo(val),  fHi(val) {}
    static SkNf Load(const T vals[N]) {
        return SkNf(SkNf<N/2,T>::Load(vals), SkNf<N/2,T>::Load(vals+N/2));
    }

    SkNf(T a, T b)                               : fLo(a),       fHi(b)       { REQUIRE(N==2); }
    SkNf(T a, T b, T c, T d)                     : fLo(a,b),     fHi(c,d)     { REQUIRE(N==4); }
    SkNf(T a, T b, T c, T d, T e, T f, T g, T h) : fLo(a,b,c,d), fHi(e,f,g,h) { REQUIRE(N==8); }

    void store(T vals[N]) const {
        fLo.store(vals);
        fHi.store(vals+N/2);
    }

    SkNf operator + (const SkNf& o) const { return SkNf(fLo + o.fLo, fHi + o.fHi); }
    SkNf operator - (const SkNf& o) const { return SkNf(fLo - o.fLo, fHi - o.fHi); }
    SkNf operator * (const SkNf& o) const { return SkNf(fLo * o.fLo, fHi * o.fHi); }
    SkNf operator / (const SkNf& o) const { return SkNf(fLo / o.fLo, fHi / o.fHi); }

    Nb operator == (const SkNf& o) const { return Nb(fLo == o.fLo, fHi == o.fHi); }
    Nb operator != (const SkNf& o) const { return Nb(fLo != o.fLo, fHi != o.fHi); }
    Nb operator  < (const SkNf& o) const { return Nb(fLo  < o.fLo, fHi  < o.fHi); }
    Nb operator  > (const SkNf& o) const { return Nb(fLo  > o.fLo, fHi  > o.fHi); }
    Nb operator <= (const SkNf& o) const { return Nb(fLo <= o.fLo, fHi <= o.fHi); }
    Nb operator >= (const SkNf& o) const { return Nb(fLo >= o.fLo, fHi >= o.fHi); }

    static SkNf Min(const SkNf& l, const SkNf& r) {
        return SkNf(SkNf<N/2,T>::Min(l.fLo, r.fLo), SkNf<N/2,T>::Min(l.fHi, r.fHi));
    }
    static SkNf Max(const SkNf& l, const SkNf& r) {
        return SkNf(SkNf<N/2,T>::Max(l.fLo, r.fLo), SkNf<N/2,T>::Max(l.fHi, r.fHi));
    }

    SkNf  sqrt() const { return SkNf(fLo. sqrt(), fHi. sqrt()); }
    SkNf rsqrt() const { return SkNf(fLo.rsqrt(), fHi.rsqrt()); }

    SkNf       invert() const { return SkNf(fLo.      invert(), fHi.      invert()); }
    SkNf approxInvert() const { return SkNf(fLo.approxInvert(), fHi.approxInvert()); }

    template <int k> T kth() const {
        SkASSERT(0 <= k && k < N);
        return k < N/2 ? fLo.template kth<k>() : fHi.template kth<k-N/2>();
    }

private:
    REQUIRE(0 == (N & (N-1)));
    SkNf(const SkNf<N/2, T>& lo, const SkNf<N/2, T>& hi) : fLo(lo), fHi(hi) {}

    SkNf<N/2, T> fLo, fHi;
};


// Bottom out the default implementations with scalars when nothing's been specialized.

template <int Bytes>
class SkNb<1, Bytes> {
public:
    SkNb() {}
    explicit SkNb(bool val) : fVal(val) {}
    bool allTrue() const { return fVal; }
    bool anyTrue() const { return fVal; }
private:
    bool fVal;
};

template <typename T>
class SkNf<1,T> {
    typedef SkNb<1, sizeof(T)> Nb;
public:
    SkNf() {}
    explicit SkNf(T val) : fVal(val) {}
    static SkNf Load(const T vals[1]) { return SkNf(vals[0]); }

    void store(T vals[1]) const { vals[0] = fVal; }

    SkNf operator + (const SkNf& o) const { return SkNf(fVal + o.fVal); }
    SkNf operator - (const SkNf& o) const { return SkNf(fVal - o.fVal); }
    SkNf operator * (const SkNf& o) const { return SkNf(fVal * o.fVal); }
    SkNf operator / (const SkNf& o) const { return SkNf(fVal / o.fVal); }

    Nb operator == (const SkNf& o) const { return Nb(fVal == o.fVal); }
    Nb operator != (const SkNf& o) const { return Nb(fVal != o.fVal); }
    Nb operator  < (const SkNf& o) const { return Nb(fVal  < o.fVal); }
    Nb operator  > (const SkNf& o) const { return Nb(fVal  > o.fVal); }
    Nb operator <= (const SkNf& o) const { return Nb(fVal <= o.fVal); }
    Nb operator >= (const SkNf& o) const { return Nb(fVal >= o.fVal); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return SkNf(SkTMin(l.fVal, r.fVal)); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return SkNf(SkTMax(l.fVal, r.fVal)); }

    SkNf  sqrt() const { return SkNf(Sqrt(fVal));        }
    SkNf rsqrt() const { return SkNf((T)1 / Sqrt(fVal)); }

    SkNf       invert() const { return SkNf((T)1 / fVal); }
    SkNf approxInvert() const { return this->invert();    }

    template <int k> T kth() const {
        SkASSERT(k == 0);
        return fVal;
    }

private:
    // We do double sqrts natively, or via floats for any other type.
    template <typename U>
    static U      Sqrt(U      val) { return (U) ::sqrtf((float)val); }
    static double Sqrt(double val) { return     ::sqrt (       val); }

    T fVal;
};


// Generic syntax sugar that should work equally well for all implementations.
template <typename T> T operator - (const T& l) { return T(0) - l; }

template <typename L, typename R> L& operator += (L& l, const R& r) { return (l = l + r); }
template <typename L, typename R> L& operator -= (L& l, const R& r) { return (l = l - r); }
template <typename L, typename R> L& operator *= (L& l, const R& r) { return (l = l * r); }
template <typename L, typename R> L& operator /= (L& l, const R& r) { return (l = l / r); }


// Include platform specific specializations if available.
#ifndef SKNX_NO_SIMD
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #include "../opts/SkNx_sse.h"
    #elif defined(SK_ARM_HAS_NEON)
        #include "../opts/SkNx_neon.h"
    #endif
#endif

#undef REQUIRE

typedef SkNf<2,    float> Sk2f;
typedef SkNf<2,   double> Sk2d;
typedef SkNf<2, SkScalar> Sk2s;

typedef SkNf<4,    float> Sk4f;
typedef SkNf<4,   double> Sk4d;
typedef SkNf<4, SkScalar> Sk4s;

#endif//SkNx_DEFINED
