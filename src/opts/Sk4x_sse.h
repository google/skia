// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

// Useful reading:
//   https://software.intel.com/sites/landingpage/IntrinsicsGuide/

#include "SkTypes.h"  // Keep this before any #ifdef for skbug.com/3362

#if defined(SK4X_PREAMBLE)
    // Code in this file may assume SSE and SSE2.
    #include <emmintrin.h>

    // It must check for later instruction sets.
    #if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
        #include <immintrin.h>
    #endif

    // A little bit of template metaprogramming to map
    // float to __m128 and int32_t to __m128i.
    template <typename T> struct SkScalarToSIMD;
    template <> struct SkScalarToSIMD<float>   { typedef __m128  Type; };
    template <> struct SkScalarToSIMD<int32_t> { typedef __m128i Type; };

    // These are all free, zero instructions.
    // MSVC insists we use _mm_castA_B(a) instead of (B)a.
    static inline __m128  as_4f(__m128i v) { return _mm_castsi128_ps(v); }
    static inline __m128  as_4f(__m128  v) { return                  v ; }
    static inline __m128i as_4i(__m128i v) { return                  v ; }
    static inline __m128i as_4i(__m128  v) { return _mm_castps_si128(v); }

#elif defined(SK4X_PRIVATE)
    // It'd be slightly faster to call _mm_cmpeq_epi32() on an unintialized register and itself,
    // but that has caused hard to debug issues when compilers recognize dealing with uninitialized
    // memory as undefined behavior that can be optimized away.
    static __m128i True()  { return _mm_set1_epi8(~0); }

    // Leaving these implicit makes the rest of the code below a bit less noisy to read.
    Sk4x(__m128i);
    Sk4x(__m128);

    Sk4x andNot(const Sk4x&) const;

    typename SkScalarToSIMD<T>::Type fVec;

#else//Method definitions.

// Helps to get these in before anything else.
template <> inline Sk4f::Sk4x(__m128i v) : fVec(as_4f(v)) {}
template <> inline Sk4f::Sk4x(__m128  v) : fVec(      v ) {}
template <> inline Sk4i::Sk4x(__m128i v) : fVec(      v ) {}
template <> inline Sk4i::Sk4x(__m128  v) : fVec(as_4i(v)) {}

// Next, methods whose implementation is the same for Sk4f and Sk4i.
template <typename T> Sk4x<T>::Sk4x() {}
template <typename T> Sk4x<T>::Sk4x(const Sk4x& other) { *this = other; }
template <typename T> Sk4x<T>& Sk4x<T>::operator=(const Sk4x<T>& other) {
    fVec = other.fVec;
    return *this;
}

// We pun in these _mm_shuffle_* methods a little to use the fastest / most available methods.
// They're all bit-preserving operations so it shouldn't matter.

template <typename T>
Sk4x<T> Sk4x<T>::zwxy() const { return _mm_shuffle_epi32(as_4i(fVec), _MM_SHUFFLE(1,0,3,2)); }

template <typename T>
Sk4x<T> Sk4x<T>::XYAB(const Sk4x<T>& a, const Sk4x<T>& b) {
    return _mm_movelh_ps(as_4f(a.fVec), as_4f(b.fVec));
}

template <typename T>
Sk4x<T> Sk4x<T>::ZWCD(const Sk4x<T>& a, const Sk4x<T>& b) {
    return _mm_movehl_ps(as_4f(b.fVec), as_4f(a.fVec));
}

// Now we'll write all Sk4f specific methods.  This M() macro will remove some noise.
#define M(...) template <> inline __VA_ARGS__ Sk4f::

M() Sk4x(float v) : fVec(_mm_set1_ps(v)) {}
M() Sk4x(float a, float b, float c, float d) : fVec(_mm_set_ps(d,c,b,a)) {}

M(Sk4f) Load       (const float fs[4]) { return _mm_loadu_ps(fs); }
M(Sk4f) LoadAligned(const float fs[4]) { return _mm_load_ps (fs); }

M(void) store       (float fs[4]) const { _mm_storeu_ps(fs, fVec); }
M(void) storeAligned(float fs[4]) const { _mm_store_ps (fs, fVec); }

template <>
M(Sk4i) reinterpret<Sk4i>() const { return as_4i(fVec); }

template <>
M(Sk4i) cast<Sk4i>() const { return _mm_cvtps_epi32(fVec); }

// We're going to try a little experiment here and skip allTrue(), anyTrue(), and bit-manipulators
// for Sk4f.  Code that calls them probably does so accidentally.
// Ask mtklein to fill these in if you really need them.

M(Sk4f) add     (const Sk4f& o) const { return _mm_add_ps(fVec, o.fVec); }
M(Sk4f) subtract(const Sk4f& o) const { return _mm_sub_ps(fVec, o.fVec); }
M(Sk4f) multiply(const Sk4f& o) const { return _mm_mul_ps(fVec, o.fVec); }
M(Sk4f) divide  (const Sk4f& o) const { return _mm_div_ps(fVec, o.fVec); }

M(Sk4f) rsqrt() const { return _mm_rsqrt_ps(fVec); }
M(Sk4f)  sqrt() const { return _mm_sqrt_ps( fVec); }

M(Sk4i) equal           (const Sk4f& o) const { return _mm_cmpeq_ps (fVec, o.fVec); }
M(Sk4i) notEqual        (const Sk4f& o) const { return _mm_cmpneq_ps(fVec, o.fVec); }
M(Sk4i) lessThan        (const Sk4f& o) const { return _mm_cmplt_ps (fVec, o.fVec); }
M(Sk4i) greaterThan     (const Sk4f& o) const { return _mm_cmpgt_ps (fVec, o.fVec); }
M(Sk4i) lessThanEqual   (const Sk4f& o) const { return _mm_cmple_ps (fVec, o.fVec); }
M(Sk4i) greaterThanEqual(const Sk4f& o) const { return _mm_cmpge_ps (fVec, o.fVec); }

M(Sk4f) Min(const Sk4f& a, const Sk4f& b) { return _mm_min_ps(a.fVec, b.fVec); }
M(Sk4f) Max(const Sk4f& a, const Sk4f& b) { return _mm_max_ps(a.fVec, b.fVec); }

// Now we'll write all the Sk4i specific methods.  Same deal for M().
#undef M
#define M(...) template <> inline __VA_ARGS__ Sk4i::

M() Sk4x(int32_t v) : fVec(_mm_set1_epi32(v)) {}
M() Sk4x(int32_t a, int32_t b, int32_t c, int32_t d) : fVec(_mm_set_epi32(d,c,b,a)) {}

M(Sk4i) Load       (const int32_t is[4]) { return _mm_loadu_si128((const __m128i*)is); }
M(Sk4i) LoadAligned(const int32_t is[4]) { return _mm_load_si128 ((const __m128i*)is); }

M(void) store       (int32_t is[4]) const { _mm_storeu_si128((__m128i*)is, fVec); }
M(void) storeAligned(int32_t is[4]) const { _mm_store_si128 ((__m128i*)is, fVec); }

template <>
M(Sk4f) reinterpret<Sk4f>() const { return as_4f(fVec); }

template <>
M(Sk4f) cast<Sk4f>() const { return _mm_cvtepi32_ps(fVec); }

M(bool) allTrue() const { return 0xf == _mm_movemask_ps(as_4f(fVec)); }
M(bool) anyTrue() const { return 0x0 != _mm_movemask_ps(as_4f(fVec)); }

M(Sk4i) bitNot() const { return _mm_xor_si128(fVec, True()); }
M(Sk4i) bitAnd(const Sk4i& o) const { return _mm_and_si128(fVec, o.fVec); }
M(Sk4i) bitOr (const Sk4i& o) const { return _mm_or_si128 (fVec, o.fVec); }

M(Sk4i) equal           (const Sk4i& o) const { return _mm_cmpeq_epi32 (fVec, o.fVec); }
M(Sk4i) lessThan        (const Sk4i& o) const { return _mm_cmplt_epi32 (fVec, o.fVec); }
M(Sk4i) greaterThan     (const Sk4i& o) const { return _mm_cmpgt_epi32 (fVec, o.fVec); }
M(Sk4i) notEqual        (const Sk4i& o) const { return this->      equal(o).bitNot();  }
M(Sk4i) lessThanEqual   (const Sk4i& o) const { return this->greaterThan(o).bitNot();  }
M(Sk4i) greaterThanEqual(const Sk4i& o) const { return this->   lessThan(o).bitNot();  }

M(Sk4i) add     (const Sk4i& o) const { return _mm_add_epi32(fVec, o.fVec); }
M(Sk4i) subtract(const Sk4i& o) const { return _mm_sub_epi32(fVec, o.fVec); }

// SSE doesn't have integer division.  Let's see how far we can get without Sk4i::divide().

// Sk4i's multiply(), Min(), and Max() all improve significantly with SSE4.1.
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    M(Sk4i) multiply(const Sk4i& o) const { return _mm_mullo_epi32(fVec, o.fVec); }
    M(Sk4i) Min(const Sk4i& a, const Sk4i& b) { return _mm_min_epi32(a.fVec, b.fVec); }
    M(Sk4i) Max(const Sk4i& a, const Sk4i& b) { return _mm_max_epi32(a.fVec, b.fVec); }
#else
    M(Sk4i) multiply(const Sk4i& o) const {
        // First 2 32->64 bit multiplies.
        __m128i mul02 = _mm_mul_epu32(fVec, o.fVec),
                mul13 = _mm_mul_epu32(_mm_srli_si128(fVec, 4), _mm_srli_si128(o.fVec, 4));
        // Now recombine the high bits of the two products.
        return _mm_unpacklo_epi32(_mm_shuffle_epi32(mul02, _MM_SHUFFLE(0,0,2,0)),
                                  _mm_shuffle_epi32(mul13, _MM_SHUFFLE(0,0,2,0)));
    }

    M(Sk4i) andNot(const Sk4i& o) const { return _mm_andnot_si128(o.fVec, fVec); }

    M(Sk4i) Min(const Sk4i& a, const Sk4i& b) {
        Sk4i less = a.lessThan(b);
        return a.bitAnd(less).bitOr(b.andNot(less));
    }
    M(Sk4i) Max(const Sk4i& a, const Sk4i& b) {
        Sk4i less = a.lessThan(b);
        return b.bitAnd(less).bitOr(a.andNot(less));
    }
#endif

#undef M

#endif//Method definitions.
