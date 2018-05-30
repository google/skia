#ifndef SkFloatBits_DEFINED
#define SkFloatBits_DEFINED

#include "stdint.h"

static inline int32_t SkSignBitTo2sCompliment(int32_t x) {
    if (x < 0) {
        x &= 0x7FFFFFFF;
        x = -x;
    }
    return x;
}

union SkFloatIntUnion {
    float   fFloat;
    int32_t fSignBitInt;
};

static inline int32_t SkFloat2Bits(float x) {
    SkFloatIntUnion data;
    data.fFloat = x;
    return data.fSignBitInt;
}

static inline int32_t SkFloatAs2sCompliment(float x) {
    return SkSignBitTo2sCompliment(SkFloat2Bits(x));
}

constexpr int32_t gFloatBits_exponent_mask = 0x7F800000;
constexpr int32_t gFloatBits_matissa_mask  = 0x007FFFFF;

static inline bool SkFloatBits_IsFinite(int32_t bits) {
    return (bits & gFloatBits_exponent_mask) != gFloatBits_exponent_mask;
}

#endif
