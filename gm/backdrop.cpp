/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlurImageFilter.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkLiteDL.h"
#include "src/core/SkLiteRecorder.h"

#include <initializer_list>

// Make a noisy (with hard-edges) background, so we can see the effect of the blur
//
static sk_sp<SkShader> make_shader(SkScalar cx, SkScalar cy, SkScalar rad) {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE, SK_ColorGREEN, SK_ColorGREEN,
        SK_ColorRED, SK_ColorRED, SK_ColorBLUE, SK_ColorBLUE, SK_ColorGREEN, SK_ColorGREEN,
    };
    constexpr int count = SK_ARRAY_COUNT(colors);
    SkScalar pos[count] = { 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6 };
    for (int i = 0; i < count; ++i) {
        pos[i] *= 1.0f/6;
    }
    return SkGradientShader::MakeSweep(cx, cy, colors, pos, count);
}

static void do_draw(SkCanvas* canvas, bool useClip, bool useHintRect) {
    SkAutoCanvasRestore acr(canvas, true);
    canvas->clipRect({0, 0, 256, 256});

    const SkScalar cx = 128, cy = 128, rad = 100;
    SkPaint p;
    p.setShader(make_shader(cx, cy, rad));
    p.setAntiAlias(true);
    canvas->drawCircle(cx, cy, rad, p);

    // now setup a saveLayer that will pull in the backdrop and blur it
    //
    const SkRect r = {cx-50, cy-50, cx+50, cy+50};
    const SkRect* drawrptr = useHintRect ? &r : nullptr;
    const SkScalar sigma = 10;
    if (useClip) {
        canvas->clipRect(r);
    }
    auto blur = SkBlurImageFilter::Make(sigma, sigma, nullptr);
    auto rec = SkCanvas::SaveLayerRec(drawrptr, nullptr, blur.get(), 0);
    canvas->saveLayer(rec);
        // draw something inside, just to demonstrate that we don't blur the new contents,
        // just the backdrop.
        p.setColor(SK_ColorYELLOW);
        p.setShader(nullptr);
        canvas->drawCircle(cx, cy, 30, p);
    canvas->restore();
}

/*
 *  Draws a 3x4 grid of sweep circles.
 *  - for a given row, each col should be identical (canvas, picture, litedl)
 *  - row:0     no-hint-rect    no-clip-rect        expect big blur (except inner circle)
 *  - row:1     no-hint-rect    clip-rect           expect small blur (except inner circle)
 *  - row:2     hint-rect       no-clip-rect        expect big blur (except inner circle)
 *  - row:3     hint-rect       clip-rect           expect small blur (except inner circle)
 *
 *  The test is that backdrop effects should be independent of the hint-rect, but should
 *  respect the clip-rect.
 */
DEF_SIMPLE_GM(backdrop_hintrect_clipping, canvas, 768, 1024) {
    for (bool useHintRect : {false, true}) {
        for (bool useClip : {false, true}) {
            SkAutoCanvasRestore acr(canvas, true);

            do_draw(canvas, useClip, useHintRect);

            SkPictureRecorder rec;
            do_draw(rec.beginRecording(256, 256), useClip, useHintRect);
            canvas->translate(256, 0);
            canvas->drawPicture(rec.finishRecordingAsPicture());

            SkLiteDL dl;
            SkLiteRecorder lite;
            lite.reset(&dl, {0, 0, 256, 256});
            do_draw(&lite, useClip, useHintRect);
            canvas->translate(256, 0);
            dl.draw(canvas);

            acr.restore();
            canvas->translate(0, 256);
        }
    }
}
