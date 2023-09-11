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
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"

sk_sp<SkBlender> SkBlenders::Arithmetic(float k1, float k2, float k3, float k4,
                                        bool enforcePremul) {
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
    static SkRuntimeEffect* gArithmeticEffect = []{
        const char prog[] =
            "uniform half4 k;"
            "uniform half pmClamp;"

            "half4 main(half4 src, half4 dst) {"
                "half4 c = saturate(k.x * src * dst + k.y * src + k.z * dst + k.w);"
                "c.rgb = min(c.rgb, max(c.a, pmClamp));"
                "return c;"
            "}"
        ;
        auto result = SkRuntimeEffect::MakeForBlender(SkString(prog));
        SkASSERTF(result.effect, "SkBlenders::Arithmetic: %s", result.errorText.c_str());
        return result.effect.release();
    }();

    const float array[] = {
        k1, k2, k3, k4,
        enforcePremul ? 0.0f : 1.0f,
    };
    return gArithmeticEffect->makeBlender(SkData::MakeWithCopy(array, sizeof(array)));
}
