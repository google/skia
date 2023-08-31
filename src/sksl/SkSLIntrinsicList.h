/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INTRINSIC_LIST_DEFINED
#define SKSL_INTRINSIC_LIST_DEFINED

#include "src/core/SkTHash.h"

#include <cstdint>
#include <initializer_list>
#include <string_view>

// A list of every intrinsic supported by SkSL.
// Using an X-Macro (https://en.wikipedia.org/wiki/X_Macro) to manage the list.
#define SKSL_INTRINSIC_LIST          \
    SKSL_INTRINSIC(abs)              \
    SKSL_INTRINSIC(acosh)            \
    SKSL_INTRINSIC(acos)             \
    SKSL_INTRINSIC(all)              \
    SKSL_INTRINSIC(any)              \
    SKSL_INTRINSIC(asinh)            \
    SKSL_INTRINSIC(asin)             \
    SKSL_INTRINSIC(atanh)            \
    SKSL_INTRINSIC(atan)             \
    SKSL_INTRINSIC(atomicAdd)        \
    SKSL_INTRINSIC(atomicLoad)       \
    SKSL_INTRINSIC(atomicStore)      \
    SKSL_INTRINSIC(bitCount)         \
    SKSL_INTRINSIC(ceil)             \
    SKSL_INTRINSIC(clamp)            \
    SKSL_INTRINSIC(cosh)             \
    SKSL_INTRINSIC(cos)              \
    SKSL_INTRINSIC(cross)            \
    SKSL_INTRINSIC(degrees)          \
    SKSL_INTRINSIC(determinant)      \
    SKSL_INTRINSIC(dFdx)             \
    SKSL_INTRINSIC(dFdy)             \
    SKSL_INTRINSIC(distance)         \
    SKSL_INTRINSIC(dot)              \
    SKSL_INTRINSIC(equal)            \
    SKSL_INTRINSIC(eval)             \
    SKSL_INTRINSIC(exp2)             \
    SKSL_INTRINSIC(exp)              \
    SKSL_INTRINSIC(faceforward)      \
    SKSL_INTRINSIC(findLSB)          \
    SKSL_INTRINSIC(findMSB)          \
    SKSL_INTRINSIC(floatBitsToInt)   \
    SKSL_INTRINSIC(floatBitsToUint)  \
    SKSL_INTRINSIC(floor)            \
    SKSL_INTRINSIC(fma)              \
    SKSL_INTRINSIC(fract)            \
    SKSL_INTRINSIC(frexp)            \
    SKSL_INTRINSIC(fromLinearSrgb)   \
    SKSL_INTRINSIC(fwidth)           \
    SKSL_INTRINSIC(greaterThanEqual) \
    SKSL_INTRINSIC(greaterThan)      \
    SKSL_INTRINSIC(intBitsToFloat)   \
    SKSL_INTRINSIC(inversesqrt)      \
    SKSL_INTRINSIC(inverse)          \
    SKSL_INTRINSIC(isinf)            \
    SKSL_INTRINSIC(isnan)            \
    SKSL_INTRINSIC(ldexp)            \
    SKSL_INTRINSIC(length)           \
    SKSL_INTRINSIC(lessThanEqual)    \
    SKSL_INTRINSIC(lessThan)         \
    SKSL_INTRINSIC(log2)             \
    SKSL_INTRINSIC(log)              \
    SKSL_INTRINSIC(matrixCompMult)   \
    SKSL_INTRINSIC(matrixInverse)    \
    SKSL_INTRINSIC(max)              \
    SKSL_INTRINSIC(min)              \
    SKSL_INTRINSIC(mix)              \
    SKSL_INTRINSIC(modf)             \
    SKSL_INTRINSIC(mod)              \
    SKSL_INTRINSIC(normalize)        \
    SKSL_INTRINSIC(notEqual)         \
    SKSL_INTRINSIC(not )             \
    SKSL_INTRINSIC(outerProduct)     \
    SKSL_INTRINSIC(packDouble2x32)   \
    SKSL_INTRINSIC(packHalf2x16)     \
    SKSL_INTRINSIC(packSnorm2x16)    \
    SKSL_INTRINSIC(packSnorm4x8)     \
    SKSL_INTRINSIC(packUnorm2x16)    \
    SKSL_INTRINSIC(packUnorm4x8)     \
    SKSL_INTRINSIC(pow)              \
    SKSL_INTRINSIC(radians)          \
    SKSL_INTRINSIC(reflect)          \
    SKSL_INTRINSIC(refract)          \
    SKSL_INTRINSIC(roundEven)        \
    SKSL_INTRINSIC(round)            \
    SKSL_INTRINSIC(sample)           \
    SKSL_INTRINSIC(sampleGrad)       \
    SKSL_INTRINSIC(sampleLod)        \
    SKSL_INTRINSIC(saturate)         \
    SKSL_INTRINSIC(sign)             \
    SKSL_INTRINSIC(sinh)             \
    SKSL_INTRINSIC(sin)              \
    SKSL_INTRINSIC(smoothstep)       \
    SKSL_INTRINSIC(sqrt)             \
    SKSL_INTRINSIC(step)             \
    SKSL_INTRINSIC(storageBarrier)   \
    SKSL_INTRINSIC(subpassLoad)      \
    SKSL_INTRINSIC(tanh)             \
    SKSL_INTRINSIC(tan)              \
    SKSL_INTRINSIC(textureHeight)    \
    SKSL_INTRINSIC(textureRead)      \
    SKSL_INTRINSIC(textureWidth)     \
    SKSL_INTRINSIC(textureWrite)     \
    SKSL_INTRINSIC(toLinearSrgb)     \
    SKSL_INTRINSIC(transpose)        \
    SKSL_INTRINSIC(trunc)            \
    SKSL_INTRINSIC(uintBitsToFloat)  \
    SKSL_INTRINSIC(unpackDouble2x32) \
    SKSL_INTRINSIC(unpackHalf2x16)   \
    SKSL_INTRINSIC(unpackSnorm2x16)  \
    SKSL_INTRINSIC(unpackSnorm4x8)   \
    SKSL_INTRINSIC(unpackUnorm2x16)  \
    SKSL_INTRINSIC(unpackUnorm4x8)   \
    SKSL_INTRINSIC(workgroupBarrier)

namespace SkSL {

// The `IntrinsicKind` enum holds every intrinsic supported by SkSL.
#define SKSL_INTRINSIC(name) k_##name##_IntrinsicKind,
enum IntrinsicKind : int8_t {
    kNotIntrinsic = -1,
    SKSL_INTRINSIC_LIST
};
#undef SKSL_INTRINSIC

// Returns a map which allows IntrinsicKind values to be looked up by name.
using IntrinsicMap = skia_private::THashMap<std::string_view, IntrinsicKind>;
const IntrinsicMap& GetIntrinsicMap();

// Looks up intrinsic functions by name.
IntrinsicKind FindIntrinsicKind(std::string_view functionName);

}

#endif
