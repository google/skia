/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKVX_DEFINED
#define SKVX_DEFINED

// skvx::Vec<N,T> are SIMD vectors of N T's, a v1.5 successor to SkNx<N,T>.
//
// This time we're leaning a bit less on platform-specific intrinsics and a bit
// more on Clang/GCC vector extensions, but still keeping the option open to
// drop in platform-specific intrinsics, actually more easily than before.
//
// We've also fixed a few of the caveats that used to make SkNx awkward to work
// with across translation units.  skvx::Vec<N,T> always has N*sizeof(T) size
// and alignof(T) alignment and is safe to use across translation units freely.


// It'd be nice to not pull in any Skia headers here, in case we want to spin this file off.
#include <cstdint>           // intXX_t
#include <cstring>           // memcpy()
#include <initializer_list>  // std::initializer_list

namespace skvx {

// All Vec have the same simple memory layout, the same as `T vec[N]`.
// This gives Vec a consistent ABI, letting them pass between files compiled with
// different instruction sets (e.g. SSE2 and AVX2) without fear of ODR violation.
template <int N, typename T>
struct Vec {
    static_assert((N & (N-1)) == 0, "N must be a power of 2.");

    T vals[N];

    Vec() = default;

    Vec(T x) {
        for (int i = 0; i < N; i++) {
            vals[i] = x;
        }
    }

    Vec(std::initializer_list<T> xs) {
        for (int i = 0; i < N; i++) {
            vals[i] = (size_t)i < xs.size() ? xs.begin()[i]
                                            : T(0);
        }
    }

    T  operator[](int i) const { return vals[i]; }
    T& operator[](int i)       { return vals[i]; }
};


#if defined(_MSC_VER)
    #define ALWAYS_INLINE __forceinline
#else
    #define ALWAYS_INLINE __attribute__((always_inline))
#endif

// Helps tamp down on the  repetitive boilerplate.
#define ___ template <int N, typename T> static inline ALWAYS_INLINE

#if defined(__GNUC__) && !defined(__clang__) && defined(__SSE__)
    // GCC warns about ABI changes when returning >= 32 byte vectors when -mavx is not enabled.
    // The functions that do that (BitPun::operator U() and vext()) are marked ALWAYS_INLINE,
    // so we can just stifle the warning.
    #pragma GCC diagnostic ignored "-Wpsabi"
#endif

// BitPun<V> holds a V and can implicitly bit-pun that V to any other equal sized type U.
template <typename V>
struct BitPun {
    V v;

    template <typename U>
    ALWAYS_INLINE operator U() const {
        static_assert(sizeof(U) == sizeof(V), "");
        U u;
        memcpy(&u, &v, sizeof(U));
        return u;
    }
};
template <typename V>
static inline ALWAYS_INLINE BitPun<V> bit_pun(const V& v) { return {v}; }

// Translate from a value type T to its corresponding Mask, the result of a comparison.
template <typename T> struct Mask { using type = T; };
template <> struct Mask<float > { using type = int32_t; };
template <> struct Mask<double> { using type = int64_t; };
template <typename T> using mask = typename Mask<T>::type;


// We have two default strategies for implementing most operations:
//    1) lean on Clang/GCC vector extensions when available;
//    2) fall back to portable implementations when not.
// At the end we can drop in platform-specific implementations that override these defaults.

#if 1 && !defined(SKNX_NO_SIMD) && (defined(__clang__) || defined(__GNUC__))

    // VExt<N,T> types have the same size as Vec<N,T> and support most operations directly.
    // N.B. VExt<N,T> alignment is N*alignof(T), stricter than Vec<N,T>'s alignof(T).

    #if defined(__clang__)
        template <int N, typename T>
        using VExt = T __attribute__((ext_vector_type(N)));

    #elif defined(__GNUC__)
        template <int N, typename T>
        struct VecHelper {
            typedef T __attribute__((vector_size(N*sizeof(T)))) type;
        };

        template <int N, typename T>
        using VExt = typename VecHelper<N,T>::type;
    #endif

    ___ VExt<N,T> vext(Vec<N,T> v) { return bit_pun(v); }

    ___ Vec<N,T> operator+(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) + vext(y)); }
    ___ Vec<N,T> operator-(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) - vext(y)); }
    ___ Vec<N,T> operator*(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) * vext(y)); }
    ___ Vec<N,T> operator/(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) / vext(y)); }

    ___ Vec<N,T> operator^(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) ^ vext(y)); }
    ___ Vec<N,T> operator&(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) & vext(y)); }
    ___ Vec<N,T> operator|(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) | vext(y)); }

    ___ Vec<N,T> operator!(Vec<N,T> x) { return bit_pun(!vext(x)); }
    ___ Vec<N,T> operator-(Vec<N,T> x) { return bit_pun(-vext(x)); }
    ___ Vec<N,T> operator~(Vec<N,T> x) { return bit_pun(~vext(x)); }

    ___ Vec<N,T> operator<<(Vec<N,T> x, int bits) { return bit_pun(vext(x) << bits); }
    ___ Vec<N,T> operator>>(Vec<N,T> x, int bits) { return bit_pun(vext(x) >> bits); }

    ___ Vec<N, mask<T>> operator==(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) == vext(y)); }
    ___ Vec<N, mask<T>> operator!=(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) != vext(y)); }
    ___ Vec<N, mask<T>> operator<=(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) <= vext(y)); }
    ___ Vec<N, mask<T>> operator>=(Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) >= vext(y)); }
    ___ Vec<N, mask<T>> operator< (Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) <  vext(y)); }
    ___ Vec<N, mask<T>> operator> (Vec<N,T> x, Vec<N,T> y) { return bit_pun(vext(x) >  vext(y)); }

#else

    // Either SKNX_NO_SIMD is defined, or Clang/GCC vector extensions are not available.
    // We'll implement things portably, in a way that should be easily autovectorizable.

    template <int N, typename T, typename Op>
    static inline auto binop(Vec<N,T> x, Vec<N,T> y, Op op) -> Vec<N, decltype(op(x[0], y[0]))> {
        using U = decltype(op(x[0], y[0]));
        Vec<N,U> r;
        for (int i = 0; i < N; i++) {
            r[i] = op(x[i], y[i]);
        }
        return r;
    }

    ___ Vec<N,T> operator+(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a+b; }); }
    ___ Vec<N,T> operator-(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a-b; }); }
    ___ Vec<N,T> operator*(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a*b; }); }
    ___ Vec<N,T> operator/(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a/b; }); }

    ___ Vec<N,T> operator^(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a^b; }); }
    ___ Vec<N,T> operator&(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a&b; }); }
    ___ Vec<N,T> operator|(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a|b; }); }

    ___ Vec<N,T> operator!(Vec<N,T> x) { return binop(x,x, [](T a, T) { return !a; }); }
    ___ Vec<N,T> operator-(Vec<N,T> x) { return binop(x,x, [](T a, T) { return -a; }); }
    ___ Vec<N,T> operator~(Vec<N,T> x) { return binop(x,x, [](T a, T) { return ~a; }); }

    ___ Vec<N,T> operator<<(Vec<N,T> x, int bits) { return binop(x,x, [bits](T a, T) { return a << bits; }); }
    ___ Vec<N,T> operator>>(Vec<N,T> x, int bits) { return binop(x,x, [bits](T a, T) { return a >> bits; }); }

    ___ Vec<N, mask<T>> operator==(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a == b ? mask<T>(~0) : mask<T>(0); }); }
    ___ Vec<N, mask<T>> operator!=(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a != b ? mask<T>(~0) : mask<T>(0); }); }
    ___ Vec<N, mask<T>> operator<=(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a <= b ? mask<T>(~0) : mask<T>(0); }); }
    ___ Vec<N, mask<T>> operator>=(Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a >= b ? mask<T>(~0) : mask<T>(0); }); }
    ___ Vec<N, mask<T>> operator< (Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a <  b ? mask<T>(~0) : mask<T>(0); }); }
    ___ Vec<N, mask<T>> operator> (Vec<N,T> x, Vec<N,T> y) { return binop(x,y, [](T a, T b) { return a >  b ? mask<T>(~0) : mask<T>(0); }); }
#endif

}  // namespace skvx

#undef ALWAYS_INLINE
#undef ___

#endif//SKVX_DEFINED
