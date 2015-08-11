
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkFloatingPoint_DEFINED
#define SkFloatingPoint_DEFINED

#include "SkTypes.h"

#include <math.h>
#include <float.h>

// For _POSIX_VERSION
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#endif

#include "SkFloatBits.h"

// C++98 cmath std::pow seems to be the earliest portable way to get float pow.
// However, on Linux including cmath undefines isfinite.
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=14608
static inline float sk_float_pow(float base, float exp) {
    return powf(base, exp);
}

static inline float sk_float_copysign(float x, float y) {
// c++11 contains a 'float copysign(float, float)' function in <cmath>.
// clang-cl reports __cplusplus for clang, not the __cplusplus vc++ version _MSC_VER would report.
#if (defined(_MSC_VER) && defined(__clang__))
#    define SK_BUILD_WITH_CLANG_CL 1
#else
#    define SK_BUILD_WITH_CLANG_CL 0
#endif
#if (!SK_BUILD_WITH_CLANG_CL && __cplusplus >= 201103L) || (_MSC_VER >= 1800)
    return copysignf(x, y);

// Posix has demanded 'float copysignf(float, float)' (from C99) since Issue 6.
#elif defined(_POSIX_VERSION) && _POSIX_VERSION >= 200112L
    return copysignf(x, y);

// Visual studio prior to 13 only has 'double _copysign(double, double)'.
#elif defined(_MSC_VER)
    return (float)_copysign(x, y);

// Otherwise convert to bits and extract sign.
#else
    int32_t xbits = SkFloat2Bits(x);
    int32_t ybits = SkFloat2Bits(y);
    return SkBits2Float((xbits & 0x7FFFFFFF) | (ybits & 0x80000000));
#endif
}

#define sk_float_sqrt(x)        sqrtf(x)
#define sk_float_sin(x)         sinf(x)
#define sk_float_cos(x)         cosf(x)
#define sk_float_tan(x)         tanf(x)
#define sk_float_floor(x)       floorf(x)
#define sk_float_ceil(x)        ceilf(x)
#ifdef SK_BUILD_FOR_MAC
#    define sk_float_acos(x)    static_cast<float>(acos(x))
#    define sk_float_asin(x)    static_cast<float>(asin(x))
#else
#    define sk_float_acos(x)    acosf(x)
#    define sk_float_asin(x)    asinf(x)
#endif
#define sk_float_atan2(y,x)     atan2f(y,x)
#define sk_float_abs(x)         fabsf(x)
#define sk_float_mod(x,y)       fmodf(x,y)
#define sk_float_exp(x)         expf(x)
#define sk_float_log(x)         logf(x)

#define sk_float_round(x) sk_float_floor((x) + 0.5f)

// can't find log2f on android, but maybe that just a tool bug?
#ifdef SK_BUILD_FOR_ANDROID
    static inline float sk_float_log2(float x) {
        const double inv_ln_2 = 1.44269504088896;
        return (float)(log(x) * inv_ln_2);
    }
#else
    #define sk_float_log2(x)        log2f(x)
#endif

#ifdef SK_BUILD_FOR_WIN
    #define sk_float_isfinite(x)    _finite(x)
    #define sk_float_isnan(x)       _isnan(x)
    static inline int sk_float_isinf(float x) {
        int32_t bits = SkFloat2Bits(x);
        return (bits << 1) == (0xFF << 24);
    }
#else
    #define sk_float_isfinite(x)    isfinite(x)
    #define sk_float_isnan(x)       isnan(x)
    #define sk_float_isinf(x)       isinf(x)
#endif

#define sk_double_isnan(a)          sk_float_isnan(a)

#ifdef SK_USE_FLOATBITS
    #define sk_float_floor2int(x)   SkFloatToIntFloor(x)
    #define sk_float_round2int(x)   SkFloatToIntRound(x)
    #define sk_float_ceil2int(x)    SkFloatToIntCeil(x)
#else
    #define sk_float_floor2int(x)   (int)sk_float_floor(x)
    #define sk_float_round2int(x)   (int)sk_float_floor((x) + 0.5f)
    #define sk_float_ceil2int(x)    (int)sk_float_ceil(x)
#endif

#define sk_double_floor(x)          floor(x)
#define sk_double_round(x)          floor((x) + 0.5)
#define sk_double_ceil(x)           ceil(x)
#define sk_double_floor2int(x)      (int)floor(x)
#define sk_double_round2int(x)      (int)floor((x) + 0.5f)
#define sk_double_ceil2int(x)       (int)ceil(x)

extern const uint32_t gIEEENotANumber;
extern const uint32_t gIEEEInfinity;
extern const uint32_t gIEEENegativeInfinity;

#define SK_FloatNaN                 (*SkTCast<const float*>(&gIEEENotANumber))
#define SK_FloatInfinity            (*SkTCast<const float*>(&gIEEEInfinity))
#define SK_FloatNegativeInfinity    (*SkTCast<const float*>(&gIEEENegativeInfinity))

// We forward declare this to break an #include cycle.
// (SkScalar -> SkFloatingPoint -> SkOpts.h -> SkXfermode -> SkColor -> SkScalar)
namespace SkOpts { extern float (*rsqrt)(float); }

// Fast, approximate inverse square root.
// Compare to name-brand "1.0f / sk_float_sqrt(x)".  Should be around 10x faster on SSE, 2x on NEON.
static inline float sk_float_rsqrt(const float x) {
// We want all this inlined, so we'll inline SIMD and just take the hit when we don't know we've got
// it at compile time.  This is going to be too fast to productively hide behind a function pointer.
//
// We do one step of Newton's method to refine the estimates in the NEON and null paths.  No
// refinement is faster, but very innacurate.  Two steps is more accurate, but slower than 1/sqrt.
//
// Optimized constants in the null path courtesy of http://rrrola.wz.cz/inv_sqrt.html
#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(x)));
#elif defined(SK_ARM_HAS_NEON)
    // Get initial estimate.
    const float32x2_t xx = vdup_n_f32(x);  // Clever readers will note we're doing everything 2x.
    float32x2_t estimate = vrsqrte_f32(xx);

    // One step of Newton's method to refine.
    const float32x2_t estimate_sq = vmul_f32(estimate, estimate);
    estimate = vmul_f32(estimate, vrsqrts_f32(xx, estimate_sq));
    return vget_lane_f32(estimate, 0);  // 1 will work fine too; the answer's in both places.
#else
    // Perhaps runtime-detected NEON, or a portable fallback.
    return SkOpts::rsqrt(x);
#endif
}

// This is the number of significant digits we can print in a string such that when we read that
// string back we get the floating point number we expect.  The minimum value C requires is 6, but
// most compilers support 9
#ifdef FLT_DECIMAL_DIG
#define SK_FLT_DECIMAL_DIG FLT_DECIMAL_DIG
#else
#define SK_FLT_DECIMAL_DIG 9
#endif

#endif
