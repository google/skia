/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixelRef.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/utils/SkRandom.h"
#include "src/effects/imagefilters/SkRuntimeImageFilter.h"
#include "tools/ToolUtils.h"

#define WIDTH 500
#define HEIGHT 500

static sk_sp<SkImageFilter> make_filter() {
    sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForShader(SkString(R"(
        uniform shader input;
        half4 main(float2 coord) {
            coord.x += sin(coord.y / 3) * 4;
            return sample(input, coord);
        }
    )")).effect;
    return SkMakeRuntimeImageFilter(std::move(effect),
                                    /*uniforms=*/nullptr,
                                    /*input=*/nullptr);
}

DEF_SIMPLE_GM_BG(rtif_distort, canvas, WIDTH, HEIGHT, SK_ColorBLACK) {
    SkPaint filterPaint;
    filterPaint.setImageFilter(make_filter());
    canvas->saveLayer(nullptr, &filterPaint);
    const char* str = "The quick brown fox jumped over the lazy dog.";
    SkRandom rand;
    SkFont font(ToolUtils::create_portable_typeface());
    for (int i = 0; i < 25; ++i) {
        int x = rand.nextULessThan(WIDTH);
        int y = rand.nextULessThan(HEIGHT);
        SkPaint paint;
        paint.setColor(ToolUtils::color_to_565(rand.nextBits(24) | 0xFF000000));
        font.setSize(rand.nextRangeScalar(0, 300));
        canvas->drawString(str, SkIntToScalar(x), SkIntToScalar(y), font, paint);
    }
    canvas->restore();
}
