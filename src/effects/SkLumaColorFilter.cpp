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
        half lum = color.r*0.2126 + color.g*0.7152 + color.b*0.0722;
   //     lum = clamp(lum, 0, 1);
        color = half4(0, 0, 0, lum);
    }
)";

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    auto [effect, err] = SkRuntimeEffect::Make(SkString(code));
    return effect ? effect->makeColorFilter(SkData::MakeEmpty()) : nullptr;
}
