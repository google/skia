/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"

sk_sp<SkColorFilter> SkLumaColorFilter::Make() {
    auto [effect, err] = SkRuntimeEffect::Make(SkString(R"(
        void main(inout half4 c) {
            c.a = clamp(c.r * 0.2126 + c.g * 0.7152 + c.b * 0.0722, 0, 1);
            c.r = c.g = c.b = 0;
        }
    )"));
    if (!effect) { SkDebugf("luma error %s\n", err.c_str()); }
    return effect ? effect->makeColorFilter(nullptr) : nullptr;
}

// SkLumaColorFilter no longer serialized 4/17/2020

#include "src/core/SkColorFilterPriv.h"

void SkColorFilterPriv::RegisterLegacyLuma() {
    SkFlattenable::Register("SkLumaColorFilter",
                            [](SkReadBuffer& buffer) -> sk_sp<SkFlattenable> {
        return SkLumaColorFilter::Make();
    });
}
