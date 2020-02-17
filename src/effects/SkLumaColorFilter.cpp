/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"

const char code[] = R"(
    void main(inout half4 color) {
        const half3 SK_ITU_BT709_LUM_COEFF = half3(0.2126, 0.7152, 0.0722);
        half luma = dot(SK_ITU_BT709_LUM_COEFF, color.rgb);
//      luma = saturate(luma);
        color = half4(0, 0, 0, luma);
    }
)";

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    auto [effect, err] = SkRuntimeEffect::Make(SkString(code));
    return effect ? effect->makeColorFilter(SkData::MakeEmpty()) : nullptr;
}
