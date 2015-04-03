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

// The default implementations of SkNi<N,T> and SkNf<N,T> just fall back on a pair of size N/2.
template <int N, typename T>
class SkNi {
public:
    // For now SkNi is a _very_ minimal sketch just to support comparison operators on SkNf.
    SkNi() {}
    SkNi(const SkNi<N/2, T>& lo, const SkNi<N/2, T>& hi) : fLo(lo), fHi(hi) {}
    bool allTrue() const { return fLo.allTrue() && fHi.allTrue(); }
    bool anyTrue() const { return fLo.anyTrue() || fHi.anyTrue(); }

private:
    REQUIRE(0 == (N & (N-1)));
    SkNi<N/2, T> fLo, fHi;
};

template <int N, typename T>
class SkNf {
    static SkNi<N,int32_t> ToNi(float);
    static SkNi<N,int64_t> ToNi(double);
    typedef decltype(ToNi(T())) Ni;
public:
    SkNf() {}
    explicit SkNf(T val)           : fLo(val),  fHi(val)      {}
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

    Ni operator == (const SkNf& o) const { return Ni(fLo == o.fLo, fHi == o.fHi); }
    Ni operator != (const SkNf& o) const { return Ni(fLo != o.fLo, fHi != o.fHi); }
    Ni operator  < (const SkNf& o) const { return Ni(fLo  < o.fLo, fHi  < o.fHi); }
    Ni operator  > (const SkNf& o) const { return Ni(fLo  > o.fLo, fHi  > o.fHi); }
    Ni operator <= (const SkNf& o) const { return Ni(fLo <= o.fLo, fHi <= o.fHi); }
    Ni operator >= (const SkNf& o) const { return Ni(fLo >= o.fLo, fHi >= o.fHi); }

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


// Bottom out the default implementation with scalars when nothing's been specialized.
template <typename T>
class SkNi<1,T> {
public:
    SkNi() {}
    explicit SkNi(T val) : fVal(val) {}
    bool allTrue() const { return (bool)fVal; }
    bool anyTrue() const { return (bool)fVal; }

private:
    T fVal;
};

template <typename T>
class SkNf<1,T> {
    static SkNi<1,int32_t> ToNi(float);
    static SkNi<1,int64_t> ToNi(double);
    typedef decltype(ToNi(T())) Ni;
public:
    SkNf() {}
    explicit SkNf(T val)           : fVal(val)     {}
    static SkNf Load(const T vals[1]) { return SkNf(vals[0]); }

    void store(T vals[1]) const { vals[0] = fVal; }

    SkNf operator + (const SkNf& o) const { return SkNf(fVal + o.fVal); }
    SkNf operator - (const SkNf& o) const { return SkNf(fVal - o.fVal); }
    SkNf operator * (const SkNf& o) const { return SkNf(fVal * o.fVal); }
    SkNf operator / (const SkNf& o) const { return SkNf(fVal / o.fVal); }

    Ni operator == (const SkNf& o) const { return Ni(fVal == o.fVal); }
    Ni operator != (const SkNf& o) const { return Ni(fVal != o.fVal); }
    Ni operator  < (const SkNf& o) const { return Ni(fVal  < o.fVal); }
    Ni operator  > (const SkNf& o) const { return Ni(fVal  > o.fVal); }
    Ni operator <= (const SkNf& o) const { return Ni(fVal <= o.fVal); }
    Ni operator >= (const SkNf& o) const { return Ni(fVal >= o.fVal); }

    static SkNf Min(const SkNf& l, const SkNf& r) { return SkNf(SkTMin(l.fVal, r.fVal)); }
    static SkNf Max(const SkNf& l, const SkNf& r) { return SkNf(SkTMax(l.fVal, r.fVal)); }

    SkNf  sqrt() const { return SkNf(Sqrt(fVal));          }
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


// Generic syntax sugar that should work equally well for all SkNi and SkNf implementations.
template <typename SkNx> SkNx operator - (const SkNx& l) { return SkNx(0) - l; }

template <typename SkNx> SkNx& operator += (SkNx& l, const SkNx& r) { return (l = l + r); }
template <typename SkNx> SkNx& operator -= (SkNx& l, const SkNx& r) { return (l = l - r); }
template <typename SkNx> SkNx& operator *= (SkNx& l, const SkNx& r) { return (l = l * r); }
template <typename SkNx> SkNx& operator /= (SkNx& l, const SkNx& r) { return (l = l / r); }


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

typedef SkNi<4, int32_t> Sk4i;

#endif//SkNx_DEFINED
