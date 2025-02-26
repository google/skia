/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkKnownRuntimeEffects_DEFINED
#define SkKnownRuntimeEffects_DEFINED

#include "include/core/SkRefCnt.h"
#include <cstdint>

class SkRuntimeEffect;

namespace SkKnownRuntimeEffects {

// We allocate the keys in blocks in the order:
//     Skia builtins
//     Skia known runtime effects
//     First party client (i.e., Chrome and Android) known runtime effects
//     unknown runtime effects (on a first come, first served basis -> unstable)
//
// WARNING: If any of these values are changed, UniqueKeys that have stably-keyed effects
// will need to be invalidated.
// TODO(b/238759147): add a revision number that can drive the invalidation.
static constexpr int kSkiaBuiltInReservedCnt = 500;
static constexpr int kSkiaKnownRuntimeEffectsReservedCnt = 500;
static constexpr int kUserDefinedKnownRuntimeEffectsReservedCnt = 100;

static constexpr int kSkiaKnownRuntimeEffectsStart = kSkiaBuiltInReservedCnt;
static constexpr int kSkiaKnownRuntimeEffectsEnd = kSkiaKnownRuntimeEffectsStart +
                                                   kSkiaKnownRuntimeEffectsReservedCnt;

static constexpr int kUserDefinedKnownRuntimeEffectsStart = kSkiaKnownRuntimeEffectsEnd;
static constexpr int kUserDefinedKnownRuntimeEffectsEnd =
        kUserDefinedKnownRuntimeEffectsStart + kUserDefinedKnownRuntimeEffectsReservedCnt;

static constexpr int kUnknownRuntimeEffectIDStart = kUserDefinedKnownRuntimeEffectsEnd;

// All six 1DBlur* stable keys must be consecutive after 1DBlurBase and
// there is no 1DBlur24 bc for large kernels we bin by a multiple of eight.
// Similarly, all six 2DBlur* stable keys must be consecutive after 2DBlurBase and
// there is no 2DBlur24 bc for large kernels we bin by a multiple of eight.
// As for the macros:
//   M(X) is for standard entries
//   M1(X) is for helper values that should be skipped in a switch statement
//   M2(X,Y) is for entries that have an initializer (Y)
#define SK_ALL_STABLEKEYS(M, M1, M2) \
    M2(Invalid, Start)      \
    M1(1DBlurBase)          \
    M2(1DBlur4, 1DBlurBase) \
    M(1DBlur8)              \
    M(1DBlur12)             \
    M(1DBlur16)             \
    M(1DBlur20)             \
    M(1DBlur28)             \
    M1(2DBlurBase)          \
    M2(2DBlur4, 2DBlurBase) \
    M(2DBlur8)              \
    M(2DBlur12)             \
    M(2DBlur16)             \
    M(2DBlur20)             \
    M(2DBlur28)             \
    M(Blend)                \
    M(Decal)                \
    M(Displacement)         \
    M(Lighting)             \
    M(LinearMorphology)     \
    M(Magnifier)            \
    M(MatrixConvUniforms)   \
    M(MatrixConvTexSm)      \
    M(MatrixConvTexLg)      \
    M(Normal)               \
    M(SparseMorphology)     \
    M(Arithmetic)           \
    M(HighContrast)         \
    M(Lerp)                 \
    M(Luma)                 \
    M(Overdraw)

// WARNING: If any of the existing values are changed, UniqueKeys that have stably-keyed effects
// will need to be invalidated. (Adding new values to the end of the enum should be fine though.)
// TODO(b/238759147): add a revision number that can drive the invalidation
enum class StableKey : uint32_t {
    kStart =   kSkiaKnownRuntimeEffectsStart,

#define M(type) k##type,
#define M1(type) k##type,
#define M2(type, initializer) k##type = k##initializer,
    SK_ALL_STABLEKEYS(M, M1, M2)
#undef M2
#undef M1
#undef M

    kLast =    kOverdraw,
};

static const int kStableKeyCnt = static_cast<int>(StableKey::kLast) -
                                 static_cast<int>(StableKey::kStart) + 1;

// kStart cannot be allowed to be zero since that is used as the not-a-stable-key value in the
// serialized runtime effects (c.f., SkRuntimeShader::flatten)
static_assert(static_cast<uint32_t>(StableKey::kStart) != 0);

static_assert(static_cast<int>(StableKey::kLast) < kSkiaKnownRuntimeEffectsEnd);

// Is 'candidate' a viable StableKey value. Note that this includes the kInvalid value.
bool IsSkiaKnownRuntimeEffect(int candidate);

bool IsUserDefinedRuntimeEffect(int candidate);

// This call provides a rough check on 'candidate's viability as a user-defined known
// run-time effect stable key. Every use of this method should be followed up with a call to
// ShaderCodeDictionary::isUserDefinedKnownRuntimeEffect - which provides tighter bounds.
bool IsViableUserDefinedKnownRuntimeEffect(int candidate);

// Validate 'candidate' before calling GetKnownRuntimeEffect
sk_sp<SkRuntimeEffect> MaybeGetKnownRuntimeEffect(uint32_t candidate);

const SkRuntimeEffect* GetKnownRuntimeEffect(StableKey);

static_assert(static_cast<int>(StableKey::kInvalid)  == static_cast<int>(StableKey::kStart));

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
