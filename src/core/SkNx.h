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
#include <limits>
#include <math.h>
#include <type_traits>

#define SI static inline

// The default SkNx<N,T> just proxies down to a pair of SkNx<N/2, T>.
template <int N, typename T>
struct SkNx {
    typedef SkNx<N/2, T> Half;

    Half fLo, fHi;

    SkNx() = default;
    SkNx(const Half& lo, const Half& hi) : fLo(lo), fHi(hi) {}

    SkNx(T v) : fLo(v), fHi(v) {}

    SkNx(T a, T b)           : fLo(a)  , fHi(b)   { static_assert(N==2, ""); }
    SkNx(T a, T b, T c, T d) : fLo(a,b), fHi(c,d) { static_assert(N==4, ""); }
    SkNx(T a, T b, T c, T d,  T e, T f, T g, T h) : fLo(a,b,c,d), fHi(e,f,g,h) {
        static_assert(N==8, "");
    }
    SkNx(T a, T b, T c, T d,  T e, T f, T g, T h,
         T i, T j, T k, T l,  T m, T n, T o, T p) : fLo(a,b,c,d, e,f,g,h), fHi(i,j,k,l, m,n,o,p) {
        static_assert(N==16, "");
    }

    T operator[](int k) const {
        SkASSERT(0 <= k && k < N);
        return k < N/2 ? fLo[k] : fHi[k-N/2];
    }

    static SkNx Load(const void* vptr) {
        auto ptr = (const char*)vptr;
        return { Half::Load(ptr), Half::Load(ptr + N/2*sizeof(T)) };
    }
    void store(void* vptr) const {
        auto ptr = (char*)vptr;
        fLo.store(ptr);
        fHi.store(ptr + N/2*sizeof(T));
    }

    bool anyTrue() const { return fLo.anyTrue() || fHi.anyTrue(); }
    bool allTrue() const { return fLo.allTrue() && fHi.allTrue(); }

    SkNx    abs() const { return { fLo.   abs(), fHi.   abs() }; }
    SkNx   sqrt() const { return { fLo.  sqrt(), fHi.  sqrt() }; }
    SkNx  rsqrt() const { return { fLo. rsqrt(), fHi. rsqrt() }; }
    SkNx  floor() const { return { fLo. floor(), fHi. floor() }; }
    SkNx invert() const { return { fLo.invert(), fHi.invert() }; }

    SkNx operator!() const { return { !fLo, !fHi }; }
    SkNx operator-() const { return { -fLo, -fHi }; }
    SkNx operator~() const { return { ~fLo, ~fHi }; }

    SkNx operator<<(int bits) const { return { fLo << bits, fHi << bits }; }
    SkNx operator>>(int bits) const { return { fLo >> bits, fHi >> bits }; }

    SkNx operator+(const SkNx& y) const { return { fLo + y.fLo, fHi + y.fHi }; }
    SkNx operator-(const SkNx& y) const { return { fLo - y.fLo, fHi - y.fHi }; }
    SkNx operator*(const SkNx& y) const { return { fLo * y.fLo, fHi * y.fHi }; }
    SkNx operator/(const SkNx& y) const { return { fLo / y.fLo, fHi / y.fHi }; }

    SkNx operator&(const SkNx& y) const { return { fLo & y.fLo, fHi & y.fHi }; }
    SkNx operator|(const SkNx& y) const { return { fLo | y.fLo, fHi | y.fHi }; }
    SkNx operator^(const SkNx& y) const { return { fLo ^ y.fLo, fHi ^ y.fHi }; }

    SkNx operator==(const SkNx& y) const { return { fLo == y.fLo, fHi == y.fHi }; }
    SkNx operator!=(const SkNx& y) const { return { fLo != y.fLo, fHi != y.fHi }; }
    SkNx operator<=(const SkNx& y) const { return { fLo <= y.fLo, fHi <= y.fHi }; }
    SkNx operator>=(const SkNx& y) const { return { fLo >= y.fLo, fHi >= y.fHi }; }
    SkNx operator< (const SkNx& y) const { return { fLo <  y.fLo, fHi <  y.fHi }; }
    SkNx operator> (const SkNx& y) const { return { fLo >  y.fLo, fHi >  y.fHi }; }

    SkNx saturatedAdd(const SkNx& y) const {
        return { fLo.saturatedAdd(y.fLo), fHi.saturatedAdd(y.fHi) };
    }
    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return { fLo.thenElse(t.fLo, e.fLo), fHi.thenElse(t.fHi, e.fHi) };
    }

    static SkNx Min(const SkNx& x, const SkNx& y) {
        return { Half::Min(x.fLo, y.fLo), Half::Min(x.fHi, y.fHi) };
    }
    static SkNx Max(const SkNx& x, const SkNx& y) {
        return { Half::Max(x.fLo, y.fLo), Half::Max(x.fHi, y.fHi) };
    }
};

// The N -> N/2 recursion bottoms out at N == 1, a scalar value.
template <typename T>
struct SkNx<1,T> {
    T fVal;

    SkNx() = default;
    SkNx(T v) : fVal(v) {}

    T operator[](int k) const {
        SkASSERT(k == 0);
        return fVal;
    }

    static SkNx Load(const void* ptr) {
        SkNx v;
        memcpy(&v, ptr, sizeof(T));
        return v;
    }
    void store(void* ptr) const { memcpy(ptr, &fVal, sizeof(T)); }

    bool anyTrue() const { return fVal != 0; }
    bool allTrue() const { return fVal != 0; }

    SkNx    abs() const { return Abs(fVal); }
    SkNx   sqrt() const { return Sqrt(fVal); }
    SkNx  rsqrt() const { return T(1) / this->sqrt(); }
    SkNx  floor() const { return Floor(fVal); }
    SkNx invert() const { return T(1) / *this; }

    SkNx operator!() const { return !fVal; }
    SkNx operator-() const { return -fVal; }
    SkNx operator~() const { return FromBits(~ToBits(fVal)); }

    SkNx operator<<(int bits) const { return fVal << bits; }
    SkNx operator>>(int bits) const { return fVal >> bits; }

    SkNx operator+(const SkNx& y) const { return fVal + y.fVal; }
    SkNx operator-(const SkNx& y) const { return fVal - y.fVal; }
    SkNx operator*(const SkNx& y) const { return fVal * y.fVal; }
    SkNx operator/(const SkNx& y) const { return fVal / y.fVal; }

    SkNx operator&(const SkNx& y) const { return FromBits(ToBits(fVal) & ToBits(y.fVal)); }
    SkNx operator|(const SkNx& y) const { return FromBits(ToBits(fVal) | ToBits(y.fVal)); }
    SkNx operator^(const SkNx& y) const { return FromBits(ToBits(fVal) ^ ToBits(y.fVal)); }

    SkNx operator==(const SkNx& y) const { return FromBits(fVal == y.fVal ? ~0 : 0); }
    SkNx operator!=(const SkNx& y) const { return FromBits(fVal != y.fVal ? ~0 : 0); }
    SkNx operator<=(const SkNx& y) const { return FromBits(fVal <= y.fVal ? ~0 : 0); }
    SkNx operator>=(const SkNx& y) const { return FromBits(fVal >= y.fVal ? ~0 : 0); }
    SkNx operator< (const SkNx& y) const { return FromBits(fVal <  y.fVal ? ~0 : 0); }
    SkNx operator> (const SkNx& y) const { return FromBits(fVal >  y.fVal ? ~0 : 0); }

    static SkNx Min(const SkNx& x, const SkNx& y) { return x.fVal < y.fVal ? x : y; }
    static SkNx Max(const SkNx& x, const SkNx& y) { return x.fVal > y.fVal ? x : y; }

    SkNx saturatedAdd(const SkNx& y) const {
        static_assert(std::is_unsigned<T>::value, "");
        T sum = fVal + y.fVal;
        return sum < fVal ? std::numeric_limits<T>::max() : sum;
    }

    SkNx thenElse(const SkNx& t, const SkNx& e) const { return fVal != 0 ? t : e; }

private:
    // Helper functions to choose the right float/double methods.  (In <cmath> madness lies...)
    static float   Abs(float val) { return  ::fabsf(val); }
    static float  Sqrt(float val) { return  ::sqrtf(val); }
    static float Floor(float val) { return ::floorf(val); }

    static double   Abs(double val) { return  ::fabs(val); }
    static double  Sqrt(double val) { return  ::sqrt(val); }
    static double Floor(double val) { return ::floor(val); }

    // Helper functions for working with floats/doubles as bit patterns.
    template <typename U> static U ToBits(U v) { return v; }
    static int32_t ToBits(float  v) { int32_t bits; memcpy(&bits, &v, sizeof(v)); return bits; }
    static int64_t ToBits(double v) { int64_t bits; memcpy(&bits, &v, sizeof(v)); return bits; }

    template <typename Bits> static T FromBits(Bits bits) {
        static_assert(std::is_pod<T   >::value &&
                      std::is_pod<Bits>::value &&
                      sizeof(T) <= sizeof(Bits), "");
        T val;
        memcpy(&val, &bits, sizeof(T));
        return val;
    }
};

// Allow scalars on the left or right of binary operators, and things like +=, &=, etc.
#define V template <int N, typename T> SI SkNx<N,T>
    V operator+ (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) +  y; }
    V operator- (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) -  y; }
    V operator* (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) *  y; }
    V operator/ (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) /  y; }
    V operator& (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) &  y; }
    V operator| (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) |  y; }
    V operator^ (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) ^  y; }
    V operator==(T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) == y; }
    V operator!=(T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) != y; }
    V operator<=(T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) <= y; }
    V operator>=(T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) >= y; }
    V operator< (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) <  y; }
    V operator> (T x, const SkNx<N,T>& y) { return SkNx<N,T>(x) >  y; }

    V operator+ (const SkNx<N,T>& x, T y) { return x +  SkNx<N,T>(y); }
    V operator- (const SkNx<N,T>& x, T y) { return x -  SkNx<N,T>(y); }
    V operator* (const SkNx<N,T>& x, T y) { return x *  SkNx<N,T>(y); }
    V operator/ (const SkNx<N,T>& x, T y) { return x /  SkNx<N,T>(y); }
    V operator& (const SkNx<N,T>& x, T y) { return x &  SkNx<N,T>(y); }
    V operator| (const SkNx<N,T>& x, T y) { return x |  SkNx<N,T>(y); }
    V operator^ (const SkNx<N,T>& x, T y) { return x ^  SkNx<N,T>(y); }
    V operator==(const SkNx<N,T>& x, T y) { return x == SkNx<N,T>(y); }
    V operator!=(const SkNx<N,T>& x, T y) { return x != SkNx<N,T>(y); }
    V operator<=(const SkNx<N,T>& x, T y) { return x <= SkNx<N,T>(y); }
    V operator>=(const SkNx<N,T>& x, T y) { return x >= SkNx<N,T>(y); }
    V operator< (const SkNx<N,T>& x, T y) { return x <  SkNx<N,T>(y); }
    V operator> (const SkNx<N,T>& x, T y) { return x >  SkNx<N,T>(y); }

    V& operator<<=(SkNx<N,T>& x, int bits) { return (x = x << bits); }
    V& operator>>=(SkNx<N,T>& x, int bits) { return (x = x >> bits); }

    V& operator +=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x + y); }
    V& operator -=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x - y); }
    V& operator *=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x * y); }
    V& operator /=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x / y); }
    V& operator &=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x & y); }
    V& operator |=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x | y); }
    V& operator ^=(SkNx<N,T>& x, const SkNx<N,T>& y) { return (x = x ^ y); }

    V& operator +=(SkNx<N,T>& x, T y) { return (x = x + SkNx<N,T>(y)); }
    V& operator -=(SkNx<N,T>& x, T y) { return (x = x - SkNx<N,T>(y)); }
    V& operator *=(SkNx<N,T>& x, T y) { return (x = x * SkNx<N,T>(y)); }
    V& operator /=(SkNx<N,T>& x, T y) { return (x = x / SkNx<N,T>(y)); }
    V& operator &=(SkNx<N,T>& x, T y) { return (x = x & SkNx<N,T>(y)); }
    V& operator |=(SkNx<N,T>& x, T y) { return (x = x | SkNx<N,T>(y)); }
    V& operator ^=(SkNx<N,T>& x, T y) { return (x = x ^ SkNx<N,T>(y)); }
#undef V

// SkNx<N,T> ~~> SkNx<N/2,T> + SkNx<N/2,T>
template <int N, typename T>
SI void SkNx_split(const SkNx<N,T>& v, SkNx<N/2,T>* lo, SkNx<N/2,T>* hi) {
    *lo = v.fLo;
    *hi = v.fHi;
}

// SkNx<N/2,T> + SkNx<N/2,T> ~~> SkNx<N,T>
template <int N, typename T>
SI SkNx<N*2,T> SkNx_join(const SkNx<N,T>& lo, const SkNx<N,T>& hi) {
    return { lo, hi };
}

// A very generic shuffle.  Can reorder, duplicate, contract, expand...
//    Sk4f v = { R,G,B,A };
//    SkNx_shuffle<2,1,0,3>(v)         ~~> {B,G,R,A}
//    SkNx_shuffle<2,1>(v)             ~~> {B,G}
//    SkNx_shuffle<2,1,2,1,2,1,2,1>(v) ~~> {B,G,B,G,B,G,B,G}
//    SkNx_shuffle<3,3,3,3>(v)         ~~> {A,A,A,A}
template <int... Ix, int N, typename T>
SI SkNx<sizeof...(Ix),T> SkNx_shuffle(const SkNx<N,T>& v) {
    return { v[Ix]... };
}

// Cast from SkNx<N, Src> to SkNx<N, Dst>, as if you called static_cast<Dst>(Src).
template <typename Dst, typename Src, int N>
SI SkNx<N,Dst> SkNx_cast(const SkNx<N,Src>& v) {
    return { SkNx_cast<Dst>(v.fLo), SkNx_cast<Dst>(v.fHi) };
}
template <typename Dst, typename Src>
SI SkNx<1,Dst> SkNx_cast(const SkNx<1,Src>& v) {
    return static_cast<Dst>(v.fVal);
}

typedef SkNx<2,     float> Sk2f;
typedef SkNx<4,     float> Sk4f;
typedef SkNx<8,     float> Sk8f;
typedef SkNx<16,    float> Sk16f;

typedef SkNx<2,  SkScalar> Sk2s;
typedef SkNx<4,  SkScalar> Sk4s;
typedef SkNx<8,  SkScalar> Sk8s;
typedef SkNx<16, SkScalar> Sk16s;

typedef SkNx<4,   uint8_t> Sk4b;
typedef SkNx<8,   uint8_t> Sk8b;
typedef SkNx<16,  uint8_t> Sk16b;

typedef SkNx<4,  uint16_t> Sk4h;
typedef SkNx<8,  uint16_t> Sk8h;
typedef SkNx<16, uint16_t> Sk16h;

typedef SkNx<4,       int> Sk4i;

// Include platform specific specializations if available.
#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/SkNx_sse.h"
#elif !defined(SKNX_NO_SIMD) && defined(SK_ARM_HAS_NEON)
    #include "../opts/SkNx_neon.h"
#endif

SI void Sk4f_ToBytes(uint8_t p[16], const Sk4f& a, const Sk4f& b, const Sk4f& c, const Sk4f& d) {
    SkNx_cast<uint8_t>(SkNx_join(SkNx_join(a,b), SkNx_join(c,d))).store(p);
}

#undef SI

#endif//SkNx_DEFINED
