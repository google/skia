/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKnownRuntimeEffects_DEFINED
#define SkKnownRuntimeEffects_DEFINED

#include "include/core/SkTypes.h"
#include <cstdint>

class SkRuntimeEffect;

namespace SkKnownRuntimeEffects {

// We allocate the keys in blocks in the order:
//     Skia builtins
//     Skia known runtime effects
//     Android known runtime effects
//     Chrome known runtime effects
//     unknown runtime effects (on a first come, first served basis -> unstable)
//
// WARNING: If any of these values are changed, UniqueKeys that have stably-keyed effects
// will need to be invalidated.
// TODO(b/238759147): add a revision number that can drive the invalidation.
static constexpr int kSkiaBuiltInReservedCnt = 500;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 500;
static constexpr int kAndroidKnownRuntimeEffectsReservedCnt = 100;
static constexpr int kChromeKnownRuntimeEffectsReservedCnt = 100;

static constexpr int kSkiaKnownRuntimeEffectsStart = kSkiaBuiltInReservedCnt;
static constexpr int kSkiaKnownRuntimeEffectsEnd = kSkiaKnownRuntimeEffectsStart +
                                                   kSkiaKnownRuntimeEffectsReservedCnt;

static constexpr int kAndroidKnownRuntimeEffectsStart = kSkiaKnownRuntimeEffectsEnd;
static constexpr int kAndroidKnownRuntimeEffectsEnd = kAndroidKnownRuntimeEffectsStart +
                                                      kAndroidKnownRuntimeEffectsReservedCnt;

static constexpr int kChromeKnownRuntimeEffectsStart = kAndroidKnownRuntimeEffectsEnd;
static constexpr int kChromeKnownRuntimeEffectsEnd = kChromeKnownRuntimeEffectsStart +
                                                     kChromeKnownRuntimeEffectsReservedCnt;

static constexpr int kUnknownRuntimeEffectIDStart = kChromeKnownRuntimeEffectsEnd;


// WARNING: If any of the existing values are changed, UniqueKeys that have stably-keyed effects
// will need to be invalidated. (Adding new values to the end of the enum should be fine though.)
// TODO(b/238759147): add a revision number that can drive the invalidation
// TODO(b/238759147): use an X macro to stringize this list and then known runtime effects could
// have a real name in the SkSL instead of all being named "KnownRuntimeEffect."
enum class StableKey : uint32_t {
    kStart =   kSkiaKnownRuntimeEffectsStart,

    kInvalid = kStart,

    // shaders
    // Binned 1D Blurs
    k1DBlurBase,
    k1DBlur4 = k1DBlurBase, // all six 1DBlur stable keys must be consecutive after the Base
    k1DBlur8,
    k1DBlur12,
    k1DBlur16,
    k1DBlur20,
    // For large kernels we bin by a multiple of eight (so no k1DBlur24)
    k1DBlur28,

    // Binned 2D Blurs
    k2DBlurBase,
    k2DBlur4 = k2DBlurBase, // all six 2DBlur stable keys must be consecutive after the Base
    k2DBlur8,
    k2DBlur12,
    k2DBlur16,
    k2DBlur20,
    // For large kernels we bin by a multiple of eight (so no k2DBlur24)
    k2DBlur28,

    kBlend,
    kDecal,
    kDisplacement,
    kLighting,
    kLinearMorphology,
    kMagnifier,
    kNormal,
    kSparseMorphology,

    // blenders
    kArithmetic,

    // color filters
    kHighContrast,
    kLerp,
    kLuma,
    kOverdraw,

    kLast =    kOverdraw,
};

static const int kStableKeyCnt = static_cast<int>(StableKey::kLast) -
                                 static_cast<int>(StableKey::kStart) + 1;

static_assert(static_cast<int>(StableKey::kLast) < kSkiaKnownRuntimeEffectsEnd);

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);

static_assert(static_cast<int>(StableKey::k1DBlur4)  == static_cast<int>(StableKey::k1DBlurBase));
static_assert(static_cast<int>(StableKey::k1DBlur8)  == static_cast<int>(StableKey::k1DBlurBase)+1);
static_assert(static_cast<int>(StableKey::k1DBlur12) == static_cast<int>(StableKey::k1DBlurBase)+2);
static_assert(static_cast<int>(StableKey::k1DBlur16) == static_cast<int>(StableKey::k1DBlurBase)+3);
static_assert(static_cast<int>(StableKey::k1DBlur20) == static_cast<int>(StableKey::k1DBlurBase)+4);
static_assert(static_cast<int>(StableKey::k1DBlur28) == static_cast<int>(StableKey::k1DBlurBase)+5);

static_assert(static_cast<int>(StableKey::k2DBlur4)  == static_cast<int>(StableKey::k2DBlurBase));
static_assert(static_cast<int>(StableKey::k2DBlur8)  == static_cast<int>(StableKey::k2DBlurBase)+1);
static_assert(static_cast<int>(StableKey::k2DBlur12) == static_cast<int>(StableKey::k2DBlurBase)+2);
static_assert(static_cast<int>(StableKey::k2DBlur16) == static_cast<int>(StableKey::k2DBlurBase)+3);
static_assert(static_cast<int>(StableKey::k2DBlur20) == static_cast<int>(StableKey::k2DBlurBase)+4);
static_assert(static_cast<int>(StableKey::k2DBlur28) == static_cast<int>(StableKey::k2DBlurBase)+5);

} // namespace SkKnownRuntimeEffects

#endif // SkKnownRuntimeEffects_DEFINED
