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
    kBlend,

    kLast =    kBlend,
};

static const int kStableKeyCnt = static_cast<int>(StableKey::kLast) -
                                 static_cast<int>(StableKey::kStart) + 1;

static_assert(static_cast<int>(StableKey::kLast) < kSkiaKnownRuntimeEffectsEnd);

SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);

} // namespace SkKnownRuntimeEffects

#endif // SkKnownRuntimeEffects_DEFINED
