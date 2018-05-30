#ifndef SkFloatingPoint_DEFINED
#define SkFloatingPoint_DEFINED

#include <float.h>
#include <math.h>
#include <stdint.h>
#include "SkFloatBits.h"

#define sk_float_abs(x)         fabsf(x)

#ifndef _HUGE_ENUF
    #define _HUGE_ENUF  1e+300  // _HUGE_ENUF*_HUGE_ENUF must overflow
#endif

#define INFINITY   ((float)(_HUGE_ENUF * _HUGE_ENUF))

static const uint32_t kIEEENotANumber = 0x7fffffff;
#define SK_FloatNaN                 (*SkTCast<const float*>(&kIEEENotANumber))
#define SK_FloatInfinity            (+(float)INFINITY)

#if defined(__clang__) && (__clang_major__ * 1000 + __clang_minor__) >= 3007
__attribute__((no_sanitize("float-cast-overflow")))
#endif
static inline float sk_double_to_float(double x) {
    return static_cast<float>(x);
}

static inline bool sk_float_isfinite(float x) {
    return SkFloatBits_IsFinite(SkFloat2Bits(x));
}
#endif
