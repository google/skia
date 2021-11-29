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

static sk_sp<SkImageFilter> make_filter() {
    sk_sp<SkRuntimeEffect> effect = SkRuntimeEffect::MakeForShader(SkString(R"(
        uniform shader child;
        half4 main(float2 coord) {
            coord.x += sin(coord.y / 3) * 4;
            return child.eval(coord);
        }
    )")).effect;
    SkRuntimeShaderBuilder builder(std::move(effect));
    return SkImageFilters::RuntimeShader(builder, /*childShaderName=*/nullptr, /*input=*/nullptr);
}

DEF_SIMPLE_GM_BG(rtif_distort, canvas, 500, 750, SK_ColorBLACK) {
    SkRect clip = SkRect::MakeWH(250, 250);
    SkPaint filterPaint;
    filterPaint.setImageFilter(make_filter());

    auto draw_layer = [&](SkScalar tx, SkScalar ty, SkMatrix m) {
        canvas->save();
        canvas->translate(tx, ty);
        canvas->clipRect(clip);
        canvas->concat(m);
        canvas->saveLayer(nullptr, &filterPaint);
        const char* str = "The quick brown fox jumped over the lazy dog.";
        SkRandom rand;
        SkFont font(ToolUtils::create_portable_typeface());
        for (int i = 0; i < 25; ++i) {
            int x = rand.nextULessThan(500);
            int y = rand.nextULessThan(500);
            SkPaint paint;
            paint.setColor(ToolUtils::color_to_565(rand.nextBits(24) | 0xFF000000));
            font.setSize(rand.nextRangeScalar(0, 300));
            canvas->drawString(str, SkIntToScalar(x), SkIntToScalar(y), font, paint);
        }
        canvas->restore();
        canvas->restore();
    };

    draw_layer(  0,   0, SkMatrix::I());
    draw_layer(250,   0, SkMatrix::Scale(0.5f, 0.5f));
    draw_layer(  0, 250, SkMatrix::RotateDeg(45, {125, 125}));
    draw_layer(250, 250, SkMatrix::Scale(0.5f, 0.5f) * SkMatrix::RotateDeg(45, {125, 125}));
    draw_layer(  0, 500, SkMatrix::Skew(-0.5f, 0));
    SkMatrix p = SkMatrix::I();
    p.setPerspX(0.0015f);
    p.setPerspY(-0.0015f);
    draw_layer(250, 500, p);
}
