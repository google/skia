/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "include/effects/SkHighContrastFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkTPin.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkKnownRuntimeEffects.h"

#include <cfloat>

sk_sp<SkColorFilter> SkHighContrastFilter::Make(const SkHighContrastConfig& config) {
    if (!config.isValid()) {
        return nullptr;
    }

    struct Uniforms { float grayscale, invertStyle, contrast; };

    // A contrast setting of exactly +1 would divide by zero (1+c)/(1-c), so pull in to +1-ε.
    // I'm not exactly sure why we've historically pinned -1 up to -1+ε, maybe just symmetry?
    float c = SkTPin(config.fContrast,
                     -1.0f + FLT_EPSILON,
                     +1.0f - FLT_EPSILON);

    Uniforms uniforms = {
        config.fGrayscale ? 1.0f : 0.0f,
        (float)config.fInvertStyle,  // 0.0f for none, 1.0f for brightness, 2.0f for lightness
        (1+c)/(1-c),
    };

    const SkRuntimeEffect* highContrastEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kHighContrast);

    skcms_TransferFunction linear = SkNamedTransferFn::kLinear;
    SkAlphaType          unpremul = kUnpremul_SkAlphaType;
    return SkColorFilterPriv::WithWorkingFormat(
            highContrastEffect->makeColorFilter(SkData::MakeWithCopy(&uniforms,sizeof(uniforms))),
            &linear, nullptr/*use dst gamut*/, &unpremul);
}
