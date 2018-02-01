/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkShaderMaskFilter.h"

static void draw_masked_image(SkCanvas* canvas, const SkImage* image, SkScalar x, SkScalar y,
                              const SkImage* mask, sk_sp<SkMaskFilter> outer = nullptr) {
    SkMatrix matrix = SkMatrix::MakeScale(SkIntToScalar(image->width()) / mask->width(),
                                          SkIntToScalar(image->height() / mask->height()));
    SkPaint paint;
    auto mf = SkShaderMaskFilter::Make(mask->makeShader(&matrix));
    if (outer) {
        mf = SkMaskFilter::MakeCompose(outer, mf);
    }
    paint.setMaskFilter(mf);
    paint.setAntiAlias(true);
    canvas->drawImage(image, x, y, &paint);
}

#include "SkGradientShader.h"
static sk_sp<SkShader> make_shader(const SkRect& r) {
    const SkPoint pts[] = {
        { r.fLeft, r.fTop }, { r.fRight, r.fBottom },
    };
    const SkColor colors[] = { 0, SK_ColorWHITE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkShader::kRepeat_TileMode);
}

DEF_SIMPLE_GM(shadermaskfilter_gradient, canvas, 512, 512) {
    SkRect r = { 0, 0, 100, 150 };
    auto shader = make_shader(r);
    auto mf = SkShaderMaskFilter::Make(shader);

    canvas->translate(20, 20);
    canvas->scale(2, 2);

    SkPaint paint;
    paint.setMaskFilter(mf);
    paint.setColor(SK_ColorRED);
    paint.setAntiAlias(true);
    canvas->drawOval(r, paint);
}

#include "Resources.h"
DEF_SIMPLE_GM(shadermaskfilter_image, canvas, 512, 512) {
    canvas->scale(1.25f, 1.25f);

    auto image = GetResourceAsImage("images/mandrill_128.png");
    auto mask = GetResourceAsImage("images/color_wheel.png");
    auto blurmf = SkBlurMaskFilter::Make(kNormal_SkBlurStyle, 5);

    canvas->drawImage(image, 10, 10, nullptr);
    canvas->drawImage(mask, 10 + image->width() + 10.f, 10, nullptr);

    draw_masked_image(canvas, image.get(), 10, 10 + image->height() + 10.f, mask.get());
    draw_masked_image(canvas, image.get(), 10 + image->width() + 10.f, 10 + image->height() + 10.f,
                      mask.get(), blurmf);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SkPictureRecorder.h"
#include "SkPath.h"

static sk_sp<SkMaskFilter> make_path_mf(const SkPath& path, unsigned alpha) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setAlpha(alpha);

    SkPictureRecorder recorder;
    recorder.beginRecording(1000, 1000)->drawPath(path, paint);
    auto shader = SkShader::MakePictureShader(recorder.finishRecordingAsPicture(),
                                              SkShader::kClamp_TileMode, SkShader::kClamp_TileMode,
                                              nullptr, nullptr);
    return SkShaderMaskFilter::Make(shader);
}

typedef void (*MakePathsProc)(const SkRect&, SkPath*, SkPath*);

const char* gCoverageName[] = {
    "union", "sect", "diff", "rev-diff", "xor"
};

DEF_SIMPLE_GM(combinemaskfilter, canvas, 565, 250) {
    const SkRect r = { 0, 0, 100, 100 };

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkPaint labelP;
    labelP.setAntiAlias(true);
    labelP.setTextSize(20);
    labelP.setTextAlign(SkPaint::kCenter_Align);

    const SkRect r2 = r.makeOutset(1.5f, 1.5f);
    SkPaint paint2;
    paint2.setStyle(SkPaint::kStroke_Style);

    auto proc0 = [](const SkRect& r, SkPath* pathA, SkPath* pathB) {
        pathA->moveTo(r.fLeft, r.fBottom);
        pathA->lineTo(r.fRight, r.fTop);
        pathA->lineTo(r.fRight, r.fBottom);
        pathB->moveTo(r.fLeft, r.fTop);
        pathB->lineTo(r.fRight, r.fBottom);
        pathB->lineTo(r.fLeft, r.fBottom);
    };
    auto proc1 = [](const SkRect& r, SkPath* pathA, SkPath* pathB) {
        pathA->addCircle(r.width()*0.25f, r.height()*0.25f, r.width()*0.5f);
        pathB->addCircle(r.width()*0.75f, r.height()*0.75f, r.width()*0.5f);
    };
    MakePathsProc procs[] = { proc0, proc1 };

    sk_sp<SkMaskFilter> mfA[2], mfB[2];
    for (int i = 0; i < 2; ++i) {
        SkPath a, b;
        procs[i](r, &a, &b);
        mfA[i] = make_path_mf(a, 1 * 0xFF / 3);
        mfB[i] = make_path_mf(b, 2 * 0xFF / 3);
    }

    canvas->translate(10, 10 + 20);
    canvas->save();
    for (int i = 0; i < 5; ++i) {
        canvas->drawText(gCoverageName[i], strlen(gCoverageName[i]), r.width()*0.5f, -10, labelP);

        SkCoverageMode mode = static_cast<SkCoverageMode>(i);
        canvas->save();
        for (int j = 0; j < 2; ++j) {
            paint.setMaskFilter(SkMaskFilter::MakeCombine(mfA[j], mfB[j], mode));
            canvas->drawRect(r2, paint2);
            canvas->drawRect(r, paint);
            canvas->translate(0, r.height() + 10);
        }
        canvas->restore();
        canvas->translate(r.width() + 10, 0);
    }
    canvas->restore();
}
