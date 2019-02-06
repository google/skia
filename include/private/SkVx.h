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
#include <algorithm>         // std::accumulate, std::copy, std::fill, std::transform, etc.
#include <cstdint>           // intXX_t
#include <cstring>           // memcpy()
#include <cmath>             // std::ceil, std::floor, std::trunc, std::round, std::sqrt, etc.
#include <functional>        // std::plus, std::minus, std::multiplies, etc.
#include <initializer_list>  // std::initializer_list

// We try to use <algorithm> and <functional> where natural so that the more
// idiosyncratic parts that can't use them stand out.  This is an experiment.

namespace skvx {

// All Vec have the same simple memory layout, the same as `T vec[N]`.
// This gives Vec a consistent ABI, letting them pass between files compiled with
// different instruction sets (e.g. SSE2 and AVX2) without fear of ODR violation.
template <int N, typename T>
struct Vec {
    static_assert((N & (N-1)) == 0, "N must be a power of 2.");

    T vals[N];

    // Methods belong here in the class declaration of Vec only if:
    //   - they must be here, like constructors or operator[];
    //   - they'll definitely never want a specialized implementation.
    // Other operations on Vec should be defined outside the type.

    Vec() = default;

    Vec(T x) { std::fill(vals,vals+N, x); }

    Vec(std::initializer_list<T> xs) : Vec(0) {
        std::copy(xs.begin(), xs.begin() + std::min(xs.size(), (size_t)N), vals);
    }

    T  operator[](int i) const { return vals[i]; }
    T& operator[](int i)       { return vals[i]; }

    static Vec Load(const void* ptr) {
        Vec v;
        memcpy(&v, ptr, sizeof(Vec));
        return v;
    }
    void store(void* ptr) const {
        memcpy(ptr, this, sizeof(Vec));
    }
};


#if defined(_MSC_VER)
    #define ALWAYS_INLINE __forceinline
#else
    #define ALWAYS_INLINE __attribute__((always_inline))
#endif

// Helps tamp down on the repetitive boilerplate.
#define ___ template <int N, typename T> static inline ALWAYS_INLINE

#if defined(__GNUC__) && !defined(__clang__) && defined(__SSE__)
    // GCC warns about ABI changes when returning >= 32 byte vectors when -mavx is not enabled.
    // The functions that do that (BitPun::operator U() and to_vext()) are marked ALWAYS_INLINE,
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
static inline ALWAYS_INLINE BitPun<V> bit_pun(V v) { return {v}; }

// Translate from a value type T to its corresponding Mask, the result of a comparison.
template <typename T> struct MaskHelper { using type = T; };
template <> struct MaskHelper<float > { using type = int32_t; };
template <> struct MaskHelper<double> { using type = int64_t; };
template <typename T> using Mask = typename MaskHelper<T>::type;


// Apply op() to each lane of one or two input vectors, returning a new vector of the results.
template <int N, typename T, typename Op>
static inline auto map(Vec<N,T> x, Op op) -> Vec<N, decltype(op(x[0]))> {
    Vec<N, decltype(op(x[0]))> results;
    std::transform(x.vals, x.vals+N, results.vals, op);
    return results;
}
template <int N, typename T, typename Op>
static inline auto map(Vec<N,T> x, Vec<N,T> y, Op op) -> Vec<N, decltype(op(x[0], y[0]))> {
    Vec<N, decltype(op(x[0], y[0]))> results;
    std::transform(x.vals, x.vals+N, y.vals, results.vals, op);
    return results;
}


// We have two default strategies for implementing most operations:
//    1) lean on Clang/GCC vector extensions when available;
//    2) fall back to portable implementations when not.
// At the end we can drop in platform-specific implementations that override these defaults.

#if !defined(SKNX_NO_SIMD) && (defined(__clang__) || defined(__GNUC__))

    // VExt<N,T> types have the same size as Vec<N,T> and support most operations directly.
    // N.B. VExt<N,T> alignment is N*alignof(T), stricter than Vec<N,T>'s alignof(T).

    #if defined(__clang__)
        template <int N, typename T>
        using VExt = T __attribute__((ext_vector_type(N)));

    #elif defined(__GNUC__)
        template <int N, typename T>
        struct VExtHelper {
            typedef T __attribute__((vector_size(N*sizeof(T)))) type;
        };

        template <int N, typename T>
        using VExt = typename VExtHelper<N,T>::type;
    #endif

    ___ VExt<N,T> to_vext(Vec<N,T> v) { return bit_pun(v); }

    ___ Vec<N,T> operator+(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) + to_vext(y)); }
    ___ Vec<N,T> operator-(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) - to_vext(y)); }
    ___ Vec<N,T> operator*(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) * to_vext(y)); }
    ___ Vec<N,T> operator/(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) / to_vext(y)); }

    ___ Vec<N,T> operator^(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) ^ to_vext(y)); }
    ___ Vec<N,T> operator&(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) & to_vext(y)); }
    ___ Vec<N,T> operator|(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) | to_vext(y)); }

    ___ Vec<N,T> operator!(Vec<N,T> x) { return bit_pun(!to_vext(x)); }
    ___ Vec<N,T> operator-(Vec<N,T> x) { return bit_pun(-to_vext(x)); }
    ___ Vec<N,T> operator~(Vec<N,T> x) { return bit_pun(~to_vext(x)); }

    ___ Vec<N,T> operator<<(Vec<N,T> x, int bits) { return bit_pun(to_vext(x) << bits); }
    ___ Vec<N,T> operator>>(Vec<N,T> x, int bits) { return bit_pun(to_vext(x) >> bits); }

    ___ Vec<N, Mask<T>> operator==(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) == to_vext(y)); }
    ___ Vec<N, Mask<T>> operator!=(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) != to_vext(y)); }
    ___ Vec<N, Mask<T>> operator<=(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) <= to_vext(y)); }
    ___ Vec<N, Mask<T>> operator>=(Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) >= to_vext(y)); }
    ___ Vec<N, Mask<T>> operator< (Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) <  to_vext(y)); }
    ___ Vec<N, Mask<T>> operator> (Vec<N,T> x, Vec<N,T> y) { return bit_pun(to_vext(x) >  to_vext(y)); }

#else

    // Either SKNX_NO_SIMD is defined, or Clang/GCC vector extensions are not available.
    // We'll implement things portably, in a way that should be easily autovectorizable.

    ___ Vec<N,T> operator+(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::plus      <T>{}); }
    ___ Vec<N,T> operator-(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::minus     <T>{}); }
    ___ Vec<N,T> operator*(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::multiplies<T>{}); }
    ___ Vec<N,T> operator/(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::divides   <T>{}); }

    ___ Vec<N,T> operator^(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::bit_xor<T>{}); }
    ___ Vec<N,T> operator&(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::bit_and<T>{}); }
    ___ Vec<N,T> operator|(Vec<N,T> x, Vec<N,T> y) { return map(x,y, std::bit_or <T>{}); }

    ___ Vec<N,T> operator!(Vec<N,T> x) { return map(x, std::logical_not<T>{}); }
    ___ Vec<N,T> operator-(Vec<N,T> x) { return map(x, std::negate     <T>{}); }
    ___ Vec<N,T> operator~(Vec<N,T> x) { return map(x, std::bit_not    <T>{}); }

    ___ Vec<N,T> operator<<(Vec<N,T> x, int bits) { return map(x, [bits](T a) { return a << bits; }); }
    ___ Vec<N,T> operator>>(Vec<N,T> x, int bits) { return map(x, [bits](T a) { return a >> bits; }); }

    ___ Vec<N, Mask<T>> operator==(Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) -> Mask<T> { return a == b ? ~0 : 0; }); }
    ___ Vec<N, Mask<T>> operator!=(Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) -> Mask<T> { return a != b ? ~0 : 0; }); }
    ___ Vec<N, Mask<T>> operator<=(Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) -> Mask<T> { return a <= b ? ~0 : 0; }); }
    ___ Vec<N, Mask<T>> operator>=(Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) -> Mask<T> { return a >= b ? ~0 : 0; }); }
    ___ Vec<N, Mask<T>> operator< (Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) -> Mask<T> { return a <  b ? ~0 : 0; }); }
    ___ Vec<N, Mask<T>> operator> (Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) -> Mask<T> { return a >  b ? ~0 : 0; }); }
#endif

// Some operations we want are not expressible with Clang/GCC vector extensions,
// so we implement them using the same approach as the alternate path above.

___ Vec<N,T> if_then_else(Vec<N,Mask<T>> cond, Vec<N,T> t, Vec<N,T> e) {
    Vec<N,Mask<T>> t_bits = bit_pun(t),
                   e_bits = bit_pun(e);
    return bit_pun( (cond & t_bits) | (~cond & e_bits) );
}

___ const T* begin(const Vec<N,T>& x) { return x.vals  ; }
___       T* begin(      Vec<N,T>& x) { return x.vals  ; }
___ const T*   end(const Vec<N,T>& x) { return x.vals+N; }
___       T*   end(      Vec<N,T>& x) { return x.vals+N; }

___ Vec<N,T> min(Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) { return std::min(a,b); }); }
___ Vec<N,T> max(Vec<N,T> x, Vec<N,T> y) { return map(x,y, [](T a, T b) { return std::max(a,b); }); }

// Scalar/vector operations just splat the scalar to a vector...
___ Vec<N,T>       operator+ (T x, Vec<N,T> y) { return Vec<N,T>(x) +  y; }
___ Vec<N,T>       operator- (T x, Vec<N,T> y) { return Vec<N,T>(x) -  y; }
___ Vec<N,T>       operator* (T x, Vec<N,T> y) { return Vec<N,T>(x) *  y; }
___ Vec<N,T>       operator/ (T x, Vec<N,T> y) { return Vec<N,T>(x) /  y; }
___ Vec<N,T>       operator^ (T x, Vec<N,T> y) { return Vec<N,T>(x) ^  y; }
___ Vec<N,T>       operator& (T x, Vec<N,T> y) { return Vec<N,T>(x) &  y; }
___ Vec<N,T>       operator| (T x, Vec<N,T> y) { return Vec<N,T>(x) |  y; }
___ Vec<N,Mask<T>> operator==(T x, Vec<N,T> y) { return Vec<N,T>(x) == y; }
___ Vec<N,Mask<T>> operator!=(T x, Vec<N,T> y) { return Vec<N,T>(x) != y; }
___ Vec<N,Mask<T>> operator<=(T x, Vec<N,T> y) { return Vec<N,T>(x) <= y; }
___ Vec<N,Mask<T>> operator>=(T x, Vec<N,T> y) { return Vec<N,T>(x) >= y; }
___ Vec<N,Mask<T>> operator< (T x, Vec<N,T> y) { return Vec<N,T>(x) <  y; }
___ Vec<N,Mask<T>> operator> (T x, Vec<N,T> y) { return Vec<N,T>(x) >  y; }
___ Vec<N,T>              min(T x, Vec<N,T> y) { return min(Vec<N,T>(x), y); }
___ Vec<N,T>              max(T x, Vec<N,T> y) { return max(Vec<N,T>(x), y); }
// ... and same deal for vector/scalar operations.
___ Vec<N,T>       operator+ (Vec<N,T> x, T y) { return x +  Vec<N,T>(y); }
___ Vec<N,T>       operator- (Vec<N,T> x, T y) { return x -  Vec<N,T>(y); }
___ Vec<N,T>       operator* (Vec<N,T> x, T y) { return x *  Vec<N,T>(y); }
___ Vec<N,T>       operator/ (Vec<N,T> x, T y) { return x /  Vec<N,T>(y); }
___ Vec<N,T>       operator^ (Vec<N,T> x, T y) { return x ^  Vec<N,T>(y); }
___ Vec<N,T>       operator& (Vec<N,T> x, T y) { return x &  Vec<N,T>(y); }
___ Vec<N,T>       operator| (Vec<N,T> x, T y) { return x |  Vec<N,T>(y); }
___ Vec<N,Mask<T>> operator==(Vec<N,T> x, T y) { return x == Vec<N,T>(y); }
___ Vec<N,Mask<T>> operator!=(Vec<N,T> x, T y) { return x != Vec<N,T>(y); }
___ Vec<N,Mask<T>> operator<=(Vec<N,T> x, T y) { return x <= Vec<N,T>(y); }
___ Vec<N,Mask<T>> operator>=(Vec<N,T> x, T y) { return x >= Vec<N,T>(y); }
___ Vec<N,Mask<T>> operator< (Vec<N,T> x, T y) { return x <  Vec<N,T>(y); }
___ Vec<N,Mask<T>> operator> (Vec<N,T> x, T y) { return x >  Vec<N,T>(y); }
___ Vec<N,T>              min(Vec<N,T> x, T y) { return min(x, Vec<N,T>(y)); }
___ Vec<N,T>              max(Vec<N,T> x, T y) { return max(x, Vec<N,T>(y)); }

// The various op= operators, for vectors...
___ Vec<N,T>& operator+=(Vec<N,T>& x, Vec<N,T> y) { return (x = x + y); }
___ Vec<N,T>& operator-=(Vec<N,T>& x, Vec<N,T> y) { return (x = x - y); }
___ Vec<N,T>& operator*=(Vec<N,T>& x, Vec<N,T> y) { return (x = x * y); }
___ Vec<N,T>& operator/=(Vec<N,T>& x, Vec<N,T> y) { return (x = x / y); }
___ Vec<N,T>& operator^=(Vec<N,T>& x, Vec<N,T> y) { return (x = x ^ y); }
___ Vec<N,T>& operator&=(Vec<N,T>& x, Vec<N,T> y) { return (x = x & y); }
___ Vec<N,T>& operator|=(Vec<N,T>& x, Vec<N,T> y) { return (x = x | y); }
// ... for scalars...
___ Vec<N,T>& operator+=(Vec<N,T>& x, T y) { return (x = x + Vec<N,T>(y)); }
___ Vec<N,T>& operator-=(Vec<N,T>& x, T y) { return (x = x - Vec<N,T>(y)); }
___ Vec<N,T>& operator*=(Vec<N,T>& x, T y) { return (x = x * Vec<N,T>(y)); }
___ Vec<N,T>& operator/=(Vec<N,T>& x, T y) { return (x = x / Vec<N,T>(y)); }
___ Vec<N,T>& operator^=(Vec<N,T>& x, T y) { return (x = x ^ Vec<N,T>(y)); }
___ Vec<N,T>& operator&=(Vec<N,T>& x, T y) { return (x = x & Vec<N,T>(y)); }
___ Vec<N,T>& operator|=(Vec<N,T>& x, T y) { return (x = x | Vec<N,T>(y)); }
// ... and for shifts.
___ Vec<N,T>& operator<<=(Vec<N,T>& x, int bits) { return (x = x << bits); }
___ Vec<N,T>& operator>>=(Vec<N,T>& x, int bits) { return (x = x >> bits); }

___ Vec<N,T>  ceil(Vec<N,T> x) { return map(x, [](T a) { return std:: ceil(a); }); }
___ Vec<N,T> floor(Vec<N,T> x) { return map(x, [](T a) { return std::floor(a); }); }
___ Vec<N,T> trunc(Vec<N,T> x) { return map(x, [](T a) { return std::trunc(a); }); }
___ Vec<N,T> round(Vec<N,T> x) { return map(x, [](T a) { return std::round(a); }); }
___ Vec<N,T>  sqrt(Vec<N,T> x) { return map(x, [](T a) { return std:: sqrt(a); }); }

___ Vec<N,T>   abs(Vec<N,T> x) { return if_then_else(x < T(0), -x, x); }
___ Vec<N,T>   rcp(Vec<N,T> x) { return T(1) / x; }
___ Vec<N,T> rsqrt(Vec<N,T> x) { return rcp(sqrt(x)); }


___ T min(Vec<N,T> x) { return *std::min_element(x.vals, x.vals+N); }
___ T max(Vec<N,T> x) { return *std::max_element(x.vals, x.vals+N); }

___ bool any(Vec<N,T> x) { return std::any_of(x.vals, x.vals+N, [](T a) { return a != Mask<T>(0); }); }
___ bool all(Vec<N,T> x) { return std::all_of(x.vals, x.vals+N, [](T a) { return a != Mask<T>(0); }); }

// Platform-specific specializations and overloads can now drop in here.

}  // namespace skvx

// Since cast() takes an extra template argument D (the type to cast to),
// argument-dependent lookup won't let us just type cast<D>(...), instead
// skvx::cast<D>(...).  That's annoying given how nice all the other methods
// are, so we'll just move this guy outside into the global namespace.
// That's pretty harmless... it still only works on skvx::Vec types.
template <typename D, int N, typename S>
static inline ALWAYS_INLINE skvx::Vec<N,D> cast(skvx::Vec<N,S> src) {
#if !defined(SKNX_NO_SIMD) && defined(__clang__)
    return skvx::bit_pun(__builtin_convertvector(skvx::to_vext(src), skvx::VExt<N,D>));
#else
    return skvx::map(src, [](S a) { return (D)a; });
#endif
}


#undef ALWAYS_INLINE
#undef ___

#endif//SKVX_DEFINED
