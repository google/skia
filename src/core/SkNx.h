/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNx_DEFINED
#define SkNx_DEFINED

//#define SKNX_NO_SIMD

#include "SkScalar.h"
#include "SkTypes.h"
#include <math.h>

// The default implementations just fall back on a pair of size N/2.
// These support the union of operations we might do to ints and floats, but
// platform specializations might support fewer (e.g. no float <<, no int /).
template <int N, typename T>
class SkNx {
public:
    SkNx() {}
    SkNx(T val) : fLo(val), fHi(val) {}

    typedef SkNx<N/2, T> Half;
    SkNx(const Half& lo, const Half& hi) : fLo(lo), fHi(hi) {}

    SkNx(T a, T b)                                : fLo(a),       fHi(b)       {}
    SkNx(T a, T b, T c, T d)                      : fLo(a,b),     fHi(c,d)     {}
    SkNx(T a, T b, T c, T d,  T e, T f, T g, T h) : fLo(a,b,c,d), fHi(e,f,g,h) {}
    SkNx(T a, T b, T c, T d,  T e, T f, T g, T h,
         T i, T j, T k, T l,  T m, T n, T o, T p) : fLo(a,b,c,d, e,f,g,h), fHi(i,j,k,l, m,n,o,p) {}

    static SkNx Load(const void* ptr) {
        auto vals = (const T*)ptr;
        return SkNx(Half::Load(vals), Half::Load(vals+N/2));
    }

    void store(void* ptr) const {
        auto vals = (T*)ptr;
        fLo.store(vals);
        fHi.store(vals+N/2);
    }

#define OP(op) SkNx operator op(const SkNx& o) const { return {fLo op o.fLo, fHi op o.fHi}; }
    OP(+) OP(-) OP(*) OP(/)
    OP(&) OP(|) OP(^)
    OP(==) OP(!=) OP(<) OP(>) OP(<=) OP(>=)
#undef OP

#define OP(op) SkNx op() const { return {fLo.op(), fHi.op()}; }
    OP(abs) OP(floor)
    OP(sqrt) OP(rsqrt0) OP(rsqrt1) OP(rsqrt2)
    OP(invert) OP(approxInvert)
#undef OP

    SkNx operator << (int bits) const { return SkNx(fLo << bits, fHi << bits); }
    SkNx operator >> (int bits) const { return SkNx(fLo >> bits, fHi >> bits); }

    SkNx saturatedAdd(const SkNx& o) const {
        return {fLo.saturatedAdd(o.fLo), fHi.saturatedAdd(o.fHi)};
    }

    static SkNx Min(const SkNx& a, const SkNx& b) {
        return {Half::Min(a.fLo, b.fLo), Half::Min(a.fHi, b.fHi)};
    }
    static SkNx Max(const SkNx& a, const SkNx& b) {
        return {Half::Max(a.fLo, b.fLo), Half::Max(a.fHi, b.fHi)};
    }

    T operator[](int k) const {
        SkASSERT(0 <= k && k < N);
        return k < N/2 ? fLo[k] : fHi[k-N/2];
    }

    bool allTrue() const { return fLo.allTrue() && fHi.allTrue(); }
    bool anyTrue() const { return fLo.anyTrue() || fHi.anyTrue(); }
    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return SkNx(fLo.thenElse(t.fLo, e.fLo), fHi.thenElse(t.fHi, e.fHi));
    }

protected:
    static_assert(0 == (N & (N-1)), "N must be a power of 2.");

    Half fLo, fHi;
};

// Bottom out the default implementations with scalars when nothing's been specialized.
template <typename T>
class SkNx<1, T> {
public:
    SkNx() {}
    SkNx(T val) : fVal(val) {}

    static SkNx Load(const void* ptr) {
        auto vals = (const T*)ptr;
        return SkNx(vals[0]);
    }

    void store(void* ptr) const {
        auto vals = (T*) ptr;
        vals[0] = fVal;
    }

#define OP(op) SkNx operator op(const SkNx& o) const { return fVal op o.fVal; }
    OP(+) OP(-) OP(*) OP(/)
    OP(&) OP(|) OP(^)
    OP(==) OP(!=) OP(<) OP(>) OP(<=) OP(>=)
#undef OP

    SkNx operator << (int bits) const { return fVal << bits; }
    SkNx operator >> (int bits) const { return fVal >> bits; }

    SkNx saturatedAdd(const SkNx& o) const {
        SkASSERT((T)(~0) > 0); // TODO: support signed T?
        T sum = fVal + o.fVal;
        return sum < fVal ? (T)(~0) : sum;
    }

    static SkNx Min(const SkNx& a, const SkNx& b) { return SkTMin(a.fVal, b.fVal); }
    static SkNx Max(const SkNx& a, const SkNx& b) { return SkTMax(a.fVal, b.fVal); }

    SkNx abs() const { return SkTAbs(fVal); }
    SkNx floor() const { return Floor(fVal); }

    SkNx  sqrt () const { return Sqrt(fVal); }
    SkNx rsqrt0() const { return this->sqrt().invert(); }
    SkNx rsqrt1() const { return this->rsqrt0(); }
    SkNx rsqrt2() const { return this->rsqrt1(); }

    SkNx       invert() const { return 1 / fVal; }
    SkNx approxInvert() const { return this->invert(); }

    T operator[](int k) const {
        SkASSERT(0 == k);
        return fVal;
    }

    bool allTrue() const { return fVal != 0; }
    bool anyTrue() const { return fVal != 0; }
    SkNx thenElse(const SkNx& t, const SkNx& e) const { return fVal != 0 ? t : e; }

protected:
    static double Floor(double val) { return ::floor (val); }
    static float  Floor(float  val) { return ::floorf(val); }
    static double Sqrt(double val) { return ::sqrt (val); }
    static float  Sqrt(float  val) { return ::sqrtf(val); }

    T fVal;
};

// This generic shuffle can be called to create any valid SkNx<N,T>.
//     Sk4f f(a,b,c,d);
//     Sk2f t = SkNx_shuffle<2,1>(f);  // ~~~> Sk2f(c,b)
//     f = SkNx_shuffle<0,1,1,0>(t);   // ~~~> Sk4f(c,b,b,c)
template <int... Ix, int N, typename T>
static inline SkNx<sizeof...(Ix), T> SkNx_shuffle(const SkNx<N,T>& src) { return { src[Ix]... }; }

// This is a generic cast between two SkNx with the same number of elements N.  E.g.
//   Sk4b bs = ...;                     // Load 4 bytes.
//   Sk4f fs = SkNx_cast<float>(bs);    // Cast each byte to a float.
//   Sk4h hs = SkNx_cast<uint16_t>(fs); // Cast each float to uint16_t.
template <typename D, typename S>
static inline SkNx<2,D> SkNx_cast(const SkNx<2,S>& src) {
    return { (D)src[0], (D)src[1] };
}

template <typename D, typename S>
static inline SkNx<4,D> SkNx_cast(const SkNx<4,S>& src) {
    return { (D)src[0], (D)src[1], (D)src[2], (D)src[3] };
}

template <typename D, typename S>
static inline SkNx<8,D> SkNx_cast(const SkNx<8,S>& src) {
    return { (D)src[0], (D)src[1], (D)src[2], (D)src[3],
             (D)src[4], (D)src[5], (D)src[6], (D)src[7] };
}

template <typename D, typename S>
static inline SkNx<16,D> SkNx_cast(const SkNx<16,S>& src) {
    return { (D)src[ 0], (D)src[ 1], (D)src[ 2], (D)src[ 3],
             (D)src[ 4], (D)src[ 5], (D)src[ 6], (D)src[ 7],
             (D)src[ 8], (D)src[ 9], (D)src[10], (D)src[11],
             (D)src[12], (D)src[13], (D)src[14], (D)src[15] };
}

typedef SkNx<2,     float> Sk2f;
typedef SkNx<4,     float> Sk4f;
typedef SkNx<2,  SkScalar> Sk2s;
typedef SkNx<4,  SkScalar> Sk4s;

typedef SkNx<4,   uint8_t> Sk4b;
typedef SkNx<16,  uint8_t> Sk16b;
typedef SkNx<4,  uint16_t> Sk4h;
typedef SkNx<16, uint16_t> Sk16h;
typedef SkNx<4,       int> Sk4i;

typedef SkNx<4, int> Sk4i;

// Include platform specific specializations if available.
#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/SkNx_sse.h"
#elif !defined(SKNX_NO_SIMD) && defined(SK_ARM_HAS_NEON)
    #include "../opts/SkNx_neon.h"
#else
    static inline
    void Sk4f_ToBytes(uint8_t p[16], const Sk4f& a, const Sk4f& b, const Sk4f& c, const Sk4f& d) {
        SkNx_cast<uint8_t>(a).store(p+ 0);
        SkNx_cast<uint8_t>(b).store(p+ 4);
        SkNx_cast<uint8_t>(c).store(p+ 8);
        SkNx_cast<uint8_t>(d).store(p+12);
    }
#endif

#endif//SkNx_DEFINED
