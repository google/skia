/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkData.h"
#include "include/effects/SkOverdrawColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkRuntimeEffectPriv.h"

sk_sp<SkColorFilter> SkOverdrawColorFilter::MakeWithSkColors(const SkColor colors[kNumColors]) {
#ifdef SK_ENABLE_SKSL
    sk_sp<SkRuntimeEffect> effect = SkMakeCachedRuntimeEffect(
            SkRuntimeEffect::MakeForColorFilter,
            R"(uniform half4 color0;
               uniform half4 color1;
               uniform half4 color2;
               uniform half4 color3;
               uniform half4 color4;
               uniform half4 color5;

               half4 main(half4 color) {
                   half alpha = 255.0 * color.a;
                   color = alpha < 0.5 ? color0
                         : alpha < 1.5 ? color1
                         : alpha < 2.5 ? color2
                         : alpha < 3.5 ? color3
                         : alpha < 4.5 ? color4 : color5;
                   return color;
               }
            )");
    if (effect) {
        auto data = SkData::MakeUninitialized(kNumColors * sizeof(SkPMColor4f));
        SkPMColor4f* premul = (SkPMColor4f*)data->writable_data();
        for (int i = 0; i < kNumColors; ++i) {
            premul[i] = SkColor4f::FromColor(colors[i]).premul();
        }
        return effect->makeColorFilter(std::move(data));
    }
    return nullptr;
#else
    // TODO(skia:12197)
    return nullptr;
#endif
}
