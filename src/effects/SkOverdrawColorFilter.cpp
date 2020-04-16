/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/core/SkUnPreMultiply.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkColorFilterPriv.h"
#include "src/core/SkReadBuffer.h"

const char* SKSL_OVERDRAW_SRC = R"(
uniform half4 color0;
uniform half4 color1;
uniform half4 color2;
uniform half4 color3;
uniform half4 color4;
uniform half4 color5;

void main(inout half4 color) {
    half alpha = 255.0 * color.a;
    color = alpha < 0.5 ? color0
          : alpha < 1.5 ? color1
          : alpha < 2.5 ? color2
          : alpha < 3.5 ? color3
          : alpha < 4.5 ? color4 : color5;
}
)";

sk_sp<SkColorFilter> SkOverdrawColorFilter::MakeWithSkColors(const SkColor colors[kNumColors]) {
    auto [effect, err] = SkRuntimeEffect::Make(SkString(SKSL_OVERDRAW_SRC));
    if (effect) {
        auto data = SkData::MakeUninitialized(kNumColors * sizeof(SkPMColor4f));
        SkPMColor4f* premul = (SkPMColor4f*)data->writable_data();
        for (int i = 0; i < kNumColors; ++i) {
            premul[i] = SkColor4f::FromColor(colors[i]).premul();
        }
        return effect->makeColorFilter(std::move(data));
    }
    return nullptr;
}

void SkColorFilterPriv::RegisterLegacyOverdraw() {
    SkFlattenable::Register("SkOverdrawColorFilter",
                            [](SkReadBuffer& buffer) -> sk_sp<SkFlattenable> {
        constexpr int N = SkOverdrawColorFilter::kNumColors;
        SkPMColor premul[N];
        size_t size = buffer.getArrayCount();
        if (!buffer.validate(size == sizeof(premul))) {
            return nullptr;
        }
        if (!buffer.readByteArray(premul, sizeof(premul))) {
            return nullptr;
        }

        SkColor colors[N];
        for (int i = 0; i < N; ++i) {
            colors[i] = SkUnPreMultiply::PMColorToColor(premul[i]);
        }
        return SkOverdrawColorFilter::MakeWithSkColors(colors);
    });
}
