/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"
#include "tools/Resources.h"

#include <stddef.h>
#include <utility>

const char* gLumaSrc = R"(
    in shader input;
    half4 main() {
        return dot(sample(input).rgb, half3(0.3, 0.6, 0.1)).000r;
    }
)";

const char* gLumaSrcWithCoords = R"(
    in shader input;
    half4 main(float2 p) {
        return dot(sample(input).rgb, half3(0.3, 0.6, 0.1)).000r;
    }
)";

DEF_SIMPLE_GM(runtimecolorfilter, canvas, 256 * 3, 256) {
    auto img = GetResourceAsImage("images/mandrill_256.png");
    canvas->drawImage(img, 0, 0, nullptr);

    for (auto src : { gLumaSrc, gLumaSrcWithCoords }) {
        sk_sp<SkRuntimeEffect> effect = std::get<0>(SkRuntimeEffect::Make(SkString(src)));
        SkASSERT(effect);
        SkPaint p;
        sk_sp<SkColorFilter> input = nullptr;
        p.setColorFilter(effect->makeColorFilter(nullptr, &input, 1));
        canvas->translate(256, 0);
        canvas->drawImage(img, 0, 0, &p);
    }
}
