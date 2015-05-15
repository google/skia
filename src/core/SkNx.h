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

protected:
    REQUIRE(0 == (N & (N-1)));
    SkNb<N/2, Bytes> fLo, fHi;
};

template <int N, typename T>
class SkNi {
public:
    SkNi() {}
    SkNi(const SkNi<N/2, T>& lo, const SkNi<N/2, T>& hi) : fLo(lo), fHi(hi) {}
    explicit SkNi(T val) : fLo(val), fHi(val) {}
    static SkNi Load(const T vals[N]) {
        return SkNi(SkNi<N/2,T>::Load(vals), SkNi<N/2,T>::Load(vals+N/2));
    }

    SkNi(T a, T b)                                : fLo(a),       fHi(b)       { REQUIRE(N==2); }
    SkNi(T a, T b, T c, T d)                      : fLo(a,b),     fHi(c,d)     { REQUIRE(N==4); }
    SkNi(T a, T b, T c, T d,  T e, T f, T g, T h) : fLo(a,b,c,d), fHi(e,f,g,h) { REQUIRE(N==8); }
    SkNi(T a, T b, T c, T d,  T e, T f, T g, T h,
         T i, T j, T k, T l,  T m, T n, T o, T p)
        : fLo(a,b,c,d, e,f,g,h), fHi(i,j,k,l, m,n,o,p) { REQUIRE(N==16); }

    void store(T vals[N]) const {
        fLo.store(vals);
        fHi.store(vals+N/2);
    }

    SkNi saturatedAdd(const SkNi& o) const {
        return SkNi(fLo.saturatedAdd(o.fLo), fHi.saturatedAdd(o.fHi));
    }

    SkNi operator + (const SkNi& o) const { return SkNi(fLo + o.fLo, fHi + o.fHi); }
    SkNi operator - (const SkNi& o) const { return SkNi(fLo - o.fLo, fHi - o.fHi); }
    SkNi operator * (const SkNi& o) const { return SkNi(fLo * o.fLo, fHi * o.fHi); }

    SkNi operator << (int bits) const { return SkNi(fLo << bits, fHi << bits); }
    SkNi operator >> (int bits) const { return SkNi(fLo >> bits, fHi >> bits); }

    static SkNi Min(const SkNi& a, const SkNi& b) {
        return SkNi(SkNi<N/2, T>::Min(a.fLo, b.fLo), SkNi<N/2, T>::Min(a.fHi, b.fHi));
    }

    // TODO: comparisons, max?

    template <int k> T kth() const {
        SkASSERT(0 <= k && k < N);
        return k < N/2 ? fLo.template kth<k>() : fHi.template kth<k-N/2>();
    }

protected:
    REQUIRE(0 == (N & (N-1)));

    SkNi<N/2, T> fLo, fHi;
};

template <int N, typename T>
class SkNf {
    typedef SkNb<N, sizeof(T)> Nb;

    static int32_t MyNi(float);
    static int64_t MyNi(double);
    typedef SkNi<N, decltype(MyNi(T()))> Ni;
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

    Ni castTrunc() const { return Ni(fLo.castTrunc(), fHi.castTrunc()); }

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

    // Generally, increasing precision, increasing cost.
    SkNf rsqrt0() const { return SkNf(fLo.rsqrt0(), fHi.rsqrt0()); }
    SkNf rsqrt1() const { return SkNf(fLo.rsqrt1(), fHi.rsqrt1()); }
    SkNf rsqrt2() const { return SkNf(fLo.rsqrt2(), fHi.rsqrt2()); }

    SkNf       invert() const { return SkNf(fLo.      invert(), fHi.      invert()); }
    SkNf approxInvert() const { return SkNf(fLo.approxInvert(), fHi.approxInvert()); }

    template <int k> T kth() const {
        SkASSERT(0 <= k && k < N);
        return k < N/2 ? fLo.template kth<k>() : fHi.template kth<k-N/2>();
    }

protected:
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
protected:
    bool fVal;
};

template <typename T>
class SkNi<1,T> {
public:
    SkNi() {}
    explicit SkNi(T val) : fVal(val) {}
    static SkNi Load(const T vals[1]) { return SkNi(vals[0]); }

    void store(T vals[1]) const { vals[0] = fVal; }

    SkNi saturatedAdd(const SkNi& o) const {
        SkASSERT((T)(~0) > 0); // TODO: support signed T
        T sum = fVal + o.fVal;
        return SkNi(sum > fVal ? sum : (T)(~0));
    }

    SkNi operator + (const SkNi& o) const { return SkNi(fVal + o.fVal); }
    SkNi operator - (const SkNi& o) const { return SkNi(fVal - o.fVal); }
    SkNi operator * (const SkNi& o) const { return SkNi(fVal * o.fVal); }

    SkNi operator << (int bits) const { return SkNi(fVal << bits); }
    SkNi operator >> (int bits) const { return SkNi(fVal >> bits); }

    static SkNi Min(const SkNi& a, const SkNi& b) { return SkNi(SkTMin(a.fVal, b.fVal)); }

    template <int k> T kth() const {
        SkASSERT(0 == k);
        return fVal;
    }

protected:
    T fVal;
};

template <typename T>
class SkNf<1,T> {
    typedef SkNb<1, sizeof(T)> Nb;

    static int32_t MyNi(float);
    static int64_t MyNi(double);
    typedef SkNi<1, decltype(MyNi(T()))> Ni;
public:
    SkNf() {}
    explicit SkNf(T val) : fVal(val) {}
    static SkNf Load(const T vals[1]) { return SkNf(vals[0]); }

    void store(T vals[1]) const { vals[0] = fVal; }

    Ni castTrunc() const { return Ni(fVal); }

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
    SkNf rsqrt0() const { return SkNf((T)1 / Sqrt(fVal)); }
    SkNf rsqrt1() const { return this->rsqrt0(); }
    SkNf rsqrt2() const { return this->rsqrt1(); }

    SkNf       invert() const { return SkNf((T)1 / fVal); }
    SkNf approxInvert() const { return this->invert();    }

    template <int k> T kth() const {
        SkASSERT(k == 0);
        return fVal;
    }

protected:
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

template <typename L> L& operator <<= (L& l, int bits) { return (l = l << bits); }
template <typename L> L& operator >>= (L& l, int bits) { return (l = l >> bits); }

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

typedef SkNi<4,  uint16_t> Sk4h;
typedef SkNi<8,  uint16_t> Sk8h;
typedef SkNi<16, uint16_t> Sk16h;

typedef SkNi<16, uint8_t> Sk16b;

typedef SkNi<4,  int32_t> Sk4i;
typedef SkNi<4, uint32_t> Sk4u;

#endif//SkNx_DEFINED
