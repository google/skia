/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlender.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlenders.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkData.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkKnownRuntimeEffects.h"

sk_sp<SkBlender> SkBlenders::Arithmetic(float k1, float k2, float k3, float k4,
                                        bool enforcePremul) {
    using namespace SkKnownRuntimeEffects;

    if (!SkScalarIsFinite(k1) ||
        !SkScalarIsFinite(k2) ||
        !SkScalarIsFinite(k3) ||
        !SkScalarIsFinite(k4)) {
        return nullptr;
    }

    // Are we nearly a SkBlendMode?
    const struct {
        float       k1, k2, k3, k4;
        SkBlendMode mode;
    } table[] = {
        { 0, 1, 0, 0, SkBlendMode::kSrc   },
        { 0, 0, 1, 0, SkBlendMode::kDst   },
        { 0, 0, 0, 0, SkBlendMode::kClear },
    };
    for (const auto& t : table) {
        if (SkScalarNearlyEqual(k1, t.k1) &&
            SkScalarNearlyEqual(k2, t.k2) &&
            SkScalarNearlyEqual(k3, t.k3) &&
            SkScalarNearlyEqual(k4, t.k4)) {
            return SkBlender::Mode(t.mode);
        }
    }

    // If we get here, we need the actual blender effect.
    const SkRuntimeEffect* arithmeticEffect = GetKnownRuntimeEffect(StableKey::kArithmetic);

    const float array[] = {
        k1, k2, k3, k4,
        enforcePremul ? 0.0f : 1.0f,
    };
    return arithmeticEffect->makeBlender(SkData::MakeWithCopy(array, sizeof(array)));
}
