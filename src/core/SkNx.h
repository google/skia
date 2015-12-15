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
#define REQUIRE(x) static_assert(x, #x)

// This file may be included multiple times by .cpp files with different flags, leading
// to different definitions.  Usually that doesn't matter because it's all inlined, but
// in Debug modes the compilers may not inline everything.  So wrap everything in an
// anonymous namespace to give each includer their own silo of this code (or the linker
// will probably pick one randomly for us, which is rarely correct).
namespace {

// The default implementations just fall back on a pair of size N/2.
// These support the union of operations we might do to ints and floats, but
// platform specializations might support fewer (e.g. no float <<, no int /).
template <int N, typename T>
class SkNx {
public:
    SkNx() {}
    SkNx(const SkNx<N/2, T>& lo, const SkNx<N/2, T>& hi) : fLo(lo), fHi(hi) {}
    SkNx(T val) : fLo(val), fHi(val) {}
    static SkNx Load(const T vals[N]) {
        return SkNx(SkNx<N/2,T>::Load(vals), SkNx<N/2,T>::Load(vals+N/2));
    }

    SkNx(T a, T b)                                : fLo(a),       fHi(b)       { REQUIRE(N==2); }
    SkNx(T a, T b, T c, T d)                      : fLo(a,b),     fHi(c,d)     { REQUIRE(N==4); }
    SkNx(T a, T b, T c, T d,  T e, T f, T g, T h) : fLo(a,b,c,d), fHi(e,f,g,h) { REQUIRE(N==8); }
    SkNx(T a, T b, T c, T d,  T e, T f, T g, T h,
         T i, T j, T k, T l,  T m, T n, T o, T p)
        : fLo(a,b,c,d, e,f,g,h), fHi(i,j,k,l, m,n,o,p) { REQUIRE(N==16); }

    void store(T vals[N]) const {
        fLo.store(vals);
        fHi.store(vals+N/2);
    }

    SkNx saturatedAdd(const SkNx& o) const {
        return SkNx(fLo.saturatedAdd(o.fLo), fHi.saturatedAdd(o.fHi));
    }

    SkNx operator + (const SkNx& o) const { return SkNx(fLo + o.fLo, fHi + o.fHi); }
    SkNx operator - (const SkNx& o) const { return SkNx(fLo - o.fLo, fHi - o.fHi); }
    SkNx operator * (const SkNx& o) const { return SkNx(fLo * o.fLo, fHi * o.fHi); }
    SkNx operator / (const SkNx& o) const { return SkNx(fLo / o.fLo, fHi / o.fHi); }

    SkNx operator << (int bits) const { return SkNx(fLo << bits, fHi << bits); }
    SkNx operator >> (int bits) const { return SkNx(fLo >> bits, fHi >> bits); }

    SkNx operator == (const SkNx& o) const { return SkNx(fLo == o.fLo, fHi == o.fHi); }
    SkNx operator != (const SkNx& o) const { return SkNx(fLo != o.fLo, fHi != o.fHi); }
    SkNx operator  < (const SkNx& o) const { return SkNx(fLo  < o.fLo, fHi  < o.fHi); }
    SkNx operator  > (const SkNx& o) const { return SkNx(fLo  > o.fLo, fHi  > o.fHi); }
    SkNx operator <= (const SkNx& o) const { return SkNx(fLo <= o.fLo, fHi <= o.fHi); }
    SkNx operator >= (const SkNx& o) const { return SkNx(fLo >= o.fLo, fHi >= o.fHi); }

    static SkNx Min(const SkNx& a, const SkNx& b) {
        return SkNx(SkNx<N/2, T>::Min(a.fLo, b.fLo), SkNx<N/2, T>::Min(a.fHi, b.fHi));
    }
    static SkNx Max(const SkNx& a, const SkNx& b) {
        return SkNx(SkNx<N/2, T>::Max(a.fLo, b.fLo), SkNx<N/2, T>::Max(a.fHi, b.fHi));
    }

    SkNx sqrt() const { return SkNx(fLo.sqrt(), fHi.sqrt()); }
    // Generally, increasing precision, increasing cost.
    SkNx rsqrt0() const { return SkNx(fLo.rsqrt0(), fHi.rsqrt0()); }
    SkNx rsqrt1() const { return SkNx(fLo.rsqrt1(), fHi.rsqrt1()); }
    SkNx rsqrt2() const { return SkNx(fLo.rsqrt2(), fHi.rsqrt2()); }

    SkNx       invert() const { return SkNx(fLo.      invert(), fHi.      invert()); }
    SkNx approxInvert() const { return SkNx(fLo.approxInvert(), fHi.approxInvert()); }

    template <int k> T kth() const {
        SkASSERT(0 <= k && k < N);
        return k < N/2 ? fLo.template kth<k>() : fHi.template kth<k-N/2>();
    }

    bool allTrue() const { return fLo.allTrue() && fHi.allTrue(); }
    bool anyTrue() const { return fLo.anyTrue() || fHi.anyTrue(); }
    SkNx thenElse(const SkNx& t, const SkNx& e) const {
        return SkNx(fLo.thenElse(t.fLo, e.fLo), fHi.thenElse(t.fHi, e.fHi));
    }

protected:
    REQUIRE(0 == (N & (N-1)));

    SkNx<N/2, T> fLo, fHi;
};

// Bottom out the default implementations with scalars when nothing's been specialized.
template <typename T>
class SkNx<1,T> {
public:
    SkNx() {}
    SkNx(T val) : fVal(val) {}
    static SkNx Load(const T vals[1]) { return SkNx(vals[0]); }

    void store(T vals[1]) const { vals[0] = fVal; }

    SkNx saturatedAdd(const SkNx& o) const {
        SkASSERT((T)(~0) > 0); // TODO: support signed T
        T sum = fVal + o.fVal;
        return SkNx(sum < fVal ? (T)(~0) : sum);
    }

    SkNx operator + (const SkNx& o) const { return SkNx(fVal + o.fVal); }
    SkNx operator - (const SkNx& o) const { return SkNx(fVal - o.fVal); }
    SkNx operator * (const SkNx& o) const { return SkNx(fVal * o.fVal); }
    SkNx operator / (const SkNx& o) const { return SkNx(fVal / o.fVal); }

    SkNx operator << (int bits) const { return SkNx(fVal << bits); }
    SkNx operator >> (int bits) const { return SkNx(fVal >> bits); }

    SkNx operator == (const SkNx& o) const { return SkNx(fVal == o.fVal); }
    SkNx operator != (const SkNx& o) const { return SkNx(fVal != o.fVal); }
    SkNx operator  < (const SkNx& o) const { return SkNx(fVal  < o.fVal); }
    SkNx operator  > (const SkNx& o) const { return SkNx(fVal  > o.fVal); }
    SkNx operator <= (const SkNx& o) const { return SkNx(fVal <= o.fVal); }
    SkNx operator >= (const SkNx& o) const { return SkNx(fVal >= o.fVal); }

    static SkNx Min(const SkNx& a, const SkNx& b) { return SkNx(SkTMin(a.fVal, b.fVal)); }
    static SkNx Max(const SkNx& a, const SkNx& b) { return SkNx(SkTMax(a.fVal, b.fVal)); }

    SkNx  sqrt () const { return SkNx(Sqrt(fVal)); }
    SkNx rsqrt0() const { return this->sqrt().invert();  }
    SkNx rsqrt1() const { return this->rsqrt0(); }
    SkNx rsqrt2() const { return this->rsqrt1(); }

    SkNx       invert() const { return SkNx(1) / SkNx(fVal); }
    SkNx approxInvert() const { return this->invert();    }

    template <int k> T kth() const {
        SkASSERT(0 == k);
        return fVal;
    }

    bool allTrue() const { return fVal != 0; }
    bool anyTrue() const { return fVal != 0; }
    SkNx thenElse(const SkNx& t, const SkNx& e) const { return fVal != 0 ? t : e; }

protected:
    static double Sqrt(double val) { return ::sqrt (val); }
    static float  Sqrt(float  val) { return ::sqrtf(val); }

    T fVal;
};

// This default implementation can be specialized by ../opts/SkNx_foo.h
// if there's a better platform-specific shuffle strategy.
template <typename Nx, int... Ix>
inline Nx SkNx_shuffle_impl(const Nx& src) { return Nx( src.template kth<Ix>()... ); }

// This generic shuffle can be called with 1 or N indices:
//     Sk4f f(a,b,c,d);
//     SkNx_shuffle<3>(f);        // ~~~> Sk4f(d,d,d,d)
//     SkNx_shuffle<2,1,0,3>(f);  // ~~~> Sk4f(c,b,a,d)
template <int... Ix, typename Nx>
inline Nx SkNx_shuffle(const Nx& src) { return SkNx_shuffle_impl<Nx, Ix...>(src); }

// A reminder alias that shuffles can be used to duplicate a single index across a vector.
template <int Ix, typename Nx>
inline Nx SkNx_dup(const Nx& src) { return SkNx_shuffle<Ix>(src); }

// This is a poor-man's std::make_index_sequence from C++14.
// I'd implement it fully, but it hurts my head.
template <int...> struct SkIntSequence {};
template <int N> struct MakeSkIntSequence;
template <> struct MakeSkIntSequence< 1> : SkIntSequence<0                                    >{};
template <> struct MakeSkIntSequence< 2> : SkIntSequence<0,1                                  >{};
template <> struct MakeSkIntSequence< 4> : SkIntSequence<0,1,2,3                              >{};
template <> struct MakeSkIntSequence< 8> : SkIntSequence<0,1,2,3,4,5,6,7                      >{};
template <> struct MakeSkIntSequence<16> : SkIntSequence<0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15>{};

// This is the default/fallback implementation for SkNx_cast.  Best to specialize SkNx_cast!
template <typename D, typename S, int N, int... Ix>
SkNx<N,D> SkNx_cast_fallback(const SkNx<N,S>& src, SkIntSequence<Ix...>) {
    return SkNx<N,D>( (D)src.template kth<Ix>()... );
}

// This is a generic cast between two SkNx with the same number of elements N.  E.g.
//   Sk4b bs = ...;                    // Load 4 bytes.
//   Sk4f fs = SkNx_cast<float>(bs);   // Cast each byte to a float.
//   Sk4i is = SkNx_cast<int>(fs);     // Cast each float to int.
// This can be specialized in ../opts/SkNx_foo.h if there's a better platform-specific cast.
template <typename D, typename S, int N>
SkNx<N,D> SkNx_cast(const SkNx<N,S>& src) {
    return SkNx_cast_fallback<D,S,N>(src, MakeSkIntSequence<N>());
}

}  // namespace

typedef SkNx<2, float> Sk2f;
typedef SkNx<4, float> Sk4f;
typedef SkNx<8, float> Sk8f;

typedef SkNx<2, double> Sk2d;
typedef SkNx<4, double> Sk4d;
typedef SkNx<8, double> Sk8d;

typedef SkNx<2, SkScalar> Sk2s;
typedef SkNx<4, SkScalar> Sk4s;
typedef SkNx<8, SkScalar> Sk8s;

typedef SkNx< 4, uint16_t> Sk4h;
typedef SkNx< 8, uint16_t> Sk8h;
typedef SkNx<16, uint16_t> Sk16h;

typedef SkNx< 4, uint8_t>  Sk4b;
typedef SkNx< 8, uint8_t>  Sk8b;
typedef SkNx<16, uint8_t>  Sk16b;

typedef SkNx<4, int> Sk4i;

// Include platform specific specializations if available.
#if !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    #include "../opts/SkNx_avx.h"
#elif !defined(SKNX_NO_SIMD) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
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

#undef REQUIRE


#endif//SkNx_DEFINED
