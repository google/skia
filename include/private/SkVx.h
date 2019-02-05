/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKVX_DEFINED
#define SKVX_DEFINED

// SkVx<N,T> are SIMD vectors of N T's, a sort of version 1.5 of SkNx<N,T>.
//
// This time we're leaning a bit less on platform-specific intrinsics and a bit
// more on Clang/GCC vector extensions, but still keeping the option open to
// drop in platform-specific intrinsics, actually more easily than before.
//
// We've also fixed a few of the caveats that used to make SkNx awkward to work
// with across translation units.  SkVx<N,T> always has N*sizeof(T) size and
// alignof(T) alignment and is safe to use across translation units freely.


// It'd be nice to not pull in any Skia headers here, in case we want to spin this file off.
#include <cstdint>      // intXX_t
#include <cstring>      // memcpy()
#include <type_traits>  // std::enable_if


// All SkVx have the same simple memory layout, N contiguous T values as if `T vec[N]`.
// This gives SkVx a consistent ABI, letting them be passed betweeen files compiled
// with different instruction sets (e.g. SSE2 and AVX2) without fear of ODR violation.
//
// To make implementing some of the operations easier, we define SkVx recursively by halves.
template <int N, typename T>
struct SkVx {
    static_assert((N & (N-1)) == 0, "N must be a power of 2.");

    SkVx<N/2, T> lo, hi;

    SkVx() = default;
    SkVx(T x) : lo(x), hi(x) {}

    T  operator[](int i) const { return i < N/2 ? lo[i] : hi[i - N/2]; }
    T& operator[](int i)       { return i < N/2 ? lo[i] : hi[i - N/2]; }
};

template <typename T>
struct SkVx<1,T> {
    T val;

    SkVx() = default;
    SkVx(T x) : val(x) {}

    T  operator[](int) const { return val; }
    T& operator[](int)       { return val; }
};

// Some helpers.
namespace {

    // Helps tamp down on the  repetitive boilerplate.
    #define ___ template <int N, typename T> static inline

    // BitPun<V> holds a V and can implicitly bit-pun that V to any other equal sized type U.
    template <typename V>
    struct BitPun {
        V v;

        template <typename U>
        operator U() const {
            static_assert(sizeof(U) == sizeof(V), "");
            U u;
            memcpy(&u, &v, sizeof(U));
            return u;
        }
    };

    // A little wrapper to infer V and make BitPun<V> easy to create.
    template <typename V>
    static inline BitPun<V> bit_pun(V v) { return {v}; }

    // Translate from a value type T to its corresponding Mask, the result of a comparison.
    template <typename T> struct Mask { using type = T; };
    template <> struct Mask<float > { using type = int32_t; };
    template <> struct Mask<double> { using type = int64_t; };

    template <typename T>
    using mask = typename Mask<T>::type;

}  // namespace


// We have two default strategies for implementing most operations:
//    1) lean on Clang/GCC vector extensions when available;
//    2) fall back to portable implementations when not.
// At the end we can drop in platform-specific implementations that override these defaults.

#if 1 && !defined(SKNX_NO_SIMD) && (defined(__clang__) || defined(__GNUC__))

    // VExt<N,T> types have the same size as SkVx<N,T> and support most operations directly.
    // N.B. VExt<N,T> alignment is N*alignof(T), stricter than SkVx<N,T>'s alignof(T).

    namespace {
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
    }  // namespace

    ___ VExt<N,T> vext(SkVx<N,T> v) { return bit_pun(v); }

    ___ SkVx<N,T> operator+(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) + vext(y)); }
    ___ SkVx<N,T> operator-(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) - vext(y)); }
    ___ SkVx<N,T> operator*(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) * vext(y)); }
    ___ SkVx<N,T> operator/(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) / vext(y)); }

    ___ SkVx<N,T> operator^(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) ^ vext(y)); }
    ___ SkVx<N,T> operator&(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) & vext(y)); }
    ___ SkVx<N,T> operator|(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) | vext(y)); }

    ___ SkVx<N,T> operator!(SkVx<N,T> x) { return bit_pun(!vext(x)); }
    ___ SkVx<N,T> operator-(SkVx<N,T> x) { return bit_pun(-vext(x)); }
    ___ SkVx<N,T> operator~(SkVx<N,T> x) { return bit_pun(~vext(x)); }

    ___ SkVx<N,T> operator<<(SkVx<N,T> x, int bits) { return bit_pun(vext(x) << bits); }
    ___ SkVx<N,T> operator>>(SkVx<N,T> x, int bits) { return bit_pun(vext(x) >> bits); }

    ___ SkVx<N, mask<T>> operator==(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) == vext(y)); }
    ___ SkVx<N, mask<T>> operator!=(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) != vext(y)); }
    ___ SkVx<N, mask<T>> operator<=(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) <= vext(y)); }
    ___ SkVx<N, mask<T>> operator>=(SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) >= vext(y)); }
    ___ SkVx<N, mask<T>> operator< (SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) <  vext(y)); }
    ___ SkVx<N, mask<T>> operator> (SkVx<N,T> x, SkVx<N,T> y) { return bit_pun(vext(x) >  vext(y)); }

#else

    // Either SKNX_NO_SIMD is defined, or Clang/GCC vector extensions are not available.
    // We'll implement things portably, in a way that should be easily autovectorizable.

    template <int N, typename T, typename Op>
    static inline
    typename std::enable_if<N != 1, SkVx<N,T>>::type binop(SkVx<N,T> x, SkVx<N,T> y, Op op) {
        x.lo = binop(x.lo, y.lo, op);
        x.hi = binop(x.hi, y.hi, op);
        return x;
    }

    template <int N, typename T, typename Op>
    static inline
    typename std::enable_if<N == 1, SkVx<N,T>>::type binop(SkVx<N,T> x, SkVx<N,T> y, Op op) {
        x.val = op(x.val, y.val);
        return x;
    }

    ___ SkVx<N,T> operator+(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a+b; }); }
    ___ SkVx<N,T> operator-(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a-b; }); }
    ___ SkVx<N,T> operator*(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a*b; }); }
    ___ SkVx<N,T> operator/(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a/b; }); }

    ___ SkVx<N,T> operator^(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a^b; }); }
    ___ SkVx<N,T> operator&(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a&b; }); }
    ___ SkVx<N,T> operator|(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a|b; }); }

    ___ SkVx<N,T> operator!(SkVx<N,T> x) { return binop(x,x, [](T a, T) { return !a; }); }
    ___ SkVx<N,T> operator-(SkVx<N,T> x) { return binop(x,x, [](T a, T) { return -a; }); }
    ___ SkVx<N,T> operator~(SkVx<N,T> x) { return binop(x,x, [](T a, T) { return ~a; }); }

    ___ SkVx<N,T> operator<<(SkVx<N,T> x, int bits) { return binop(x,x, [bits](T a, T) { return a << bits; }); }
    ___ SkVx<N,T> operator>>(SkVx<N,T> x, int bits) { return binop(x,x, [bits](T a, T) { return a >> bits; }); }

    ___ SkVx<N, mask<T>> operator==(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a == b ? ~0 : 0; }); }
    ___ SkVx<N, mask<T>> operator!=(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a != b ? ~0 : 0; }); }
    ___ SkVx<N, mask<T>> operator<=(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a <= b ? ~0 : 0; }); }
    ___ SkVx<N, mask<T>> operator>=(SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a >= b ? ~0 : 0; }); }
    ___ SkVx<N, mask<T>> operator< (SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a <  b ? ~0 : 0; }); }
    ___ SkVx<N, mask<T>> operator> (SkVx<N,T> x, SkVx<N,T> y) { return binop(x,y, [](T a, T b) { return a >  b ? ~0 : 0; }); }
#endif

#endif//SKVX_DEFINED
