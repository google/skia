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
    void main(inout half4 color) {
        color.a = color.r*0.3 + color.g*0.6 + color.b*0.1;
        color.r = 0;
        color.g = 0;
        color.b = 0;
    }
)";

const char* gLumaSrcWithCoords = R"(
    void main(float2 p, inout half4 color) {
        color.a = color.r*0.3 + color.g*0.6 + color.b*0.1;
        color.r = 0;
        color.g = 0;
        color.b = 0;
    }
)";

DEF_SIMPLE_GM(runtimecolorfilter, canvas, 256 * 3, 256) {
    auto img = GetResourceAsImage("images/mandrill_256.png");
    canvas->drawImage(img, 0, 0, nullptr);

    for (auto src : { gLumaSrc, gLumaSrcWithCoords }) {
        sk_sp<SkRuntimeEffect> effect = std::get<0>(SkRuntimeEffect::Make(SkString(src)));
        SkASSERT(effect);
        SkPaint p;
        p.setColorFilter(effect->makeColorFilter(nullptr));
        canvas->translate(256, 0);
        canvas->drawImage(img, 0, 0, &p);
    }
}
