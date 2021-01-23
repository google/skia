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

const char* gNoop = R"(
    uniform shader input;
    half4 main() {
        return sample(input);
    }
)";

const char* gLumaSrc = R"(
    uniform shader input;
    half4 main() {
        return dot(sample(input).rgb, half3(0.3, 0.6, 0.1)).000r;
    }
)";

const char* gLumaSrcWithCoords = R"(
    uniform shader input;
    half4 main(float2 p) {
        return dot(sample(input).rgb, half3(0.3, 0.6, 0.1)).000r;
    }
)";

// Build up the same effect with increasingly complex control flow syntax.
// All of these are semantically equivalent and can be reduced in principle to one basic block.

// Simplest to run; hardest to write?
const char* gTernary = R"(
    uniform shader input;
    half4 main() {
        half4 color = sample(input);
        half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

        half scale = luma < 0.33333 ? 0.5
                   : luma < 0.66666 ? (0.166666 + 2.0 * (luma - 0.33333)) / luma
                   :   /* else */     (0.833333 + 0.5 * (luma - 0.66666)) / luma;
        return half4(color.rgb * scale, color.a);
    }
)";

// Uses conditional if statements but no early return.
const char* gIfs = R"(
    uniform shader input;
    half4 main() {
        half4 color = sample(input);
        half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

        half scale = 0;
        if (luma < 0.33333) {
            scale = 0.5;
        } else if (luma < 0.66666) {
            scale = (0.166666 + 2.0 * (luma - 0.33333)) / luma;
        } else {
            scale = (0.833333 + 0.5 * (luma - 0.66666)) / luma;
        }
        return half4(color.rgb * scale, color.a);
    }
)";

// Distilled from AOSP tone mapping shaders, more like what people tend to write.
const char* gEarlyReturn = R"(
    uniform shader input;
    half4 main() {
        half4 color = sample(input);
        half luma = dot(color.rgb, half3(0.3, 0.6, 0.1));

        half scale = 0;
        if (luma < 0.33333) {
            return half4(color.rgb * 0.5, color.a);
        } else if (luma < 0.66666) {
            scale = 0.166666 + 2.0 * (luma - 0.33333);
        } else {
            scale = 0.833333 + 0.5 * (luma - 0.66666);
        }
        return half4(color.rgb * (scale/luma), color.a);
    }
)";


DEF_SIMPLE_GM(runtimecolorfilter, canvas, 256 * 3, 256 * 2) {
    sk_sp<SkImage> img = GetResourceAsImage("images/mandrill_256.png");

    auto draw_filter = [&](const char* src) {
        auto [effect, err] = SkRuntimeEffect::Make(SkString(src));
        if (!effect) {
            SkDebugf("%s\n%s\n", src, err.c_str());
        }
        SkASSERT(effect);
        SkPaint p;
        sk_sp<SkColorFilter> input = nullptr;
        p.setColorFilter(effect->makeColorFilter(nullptr, &input, 1));
        canvas->drawImage(img, 0, 0, SkSamplingOptions(), &p);
        canvas->translate(256, 0);
    };

    for (const char* src : { gNoop, gLumaSrc, gLumaSrcWithCoords}) {
        draw_filter(src);
    }
    canvas->translate(-256*3, 256);
    for (const char* src : { gTernary, gIfs, gEarlyReturn}) {
        draw_filter(src);
    }
}
