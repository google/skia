/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkCoverageMode.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkBlendModePriv.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <initializer_list>

static void draw_masked_image(SkCanvas* canvas, const SkImage* image, SkScalar x, SkScalar y,
                              const SkImage* mask, sk_sp<SkMaskFilter> outer, SkBlendMode mode) {
    SkMatrix matrix = SkMatrix::MakeScale(SkIntToScalar(image->width()) / mask->width(),
                                          SkIntToScalar(image->height() / mask->height()));
    // The geometry of the drawImage is also translated by (x,y) so make the mask filter's
    // coordinate system align with the rendered rectangle.
    matrix.postTranslate(x, y);
    SkPaint paint;
    auto mf = SkShaderMaskFilter::Make(mask->makeShader(&matrix));
    if (outer) {
        mf = SkMaskFilter::MakeCompose(std::move(outer), std::move(mf));
    }
    paint.setMaskFilter(mf);
    paint.setAntiAlias(true);
    paint.setBlendMode(mode);
    canvas->drawImage(image, x, y, &paint);
}

static sk_sp<SkShader> make_shader(const SkRect& r) {
    const SkPoint pts[] = {
        { r.fLeft, r.fTop }, { r.fRight, r.fBottom },
    };
    const SkColor colors[] = { 0, SK_ColorWHITE };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kRepeat);
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

DEF_SIMPLE_GM_CAN_FAIL(shadermaskfilter_image, canvas, errorMsg, 560, 370) {
    canvas->scale(1.25f, 1.25f);

    auto image = GetResourceAsImage("images/mandrill_128.png");
    auto mask = GetResourceAsImage("images/color_wheel.png");
    if (!image || !mask) {
        *errorMsg = "Could not load images. Did you forget to set the resourcePath?";
        return skiagm::DrawResult::kFail;
    }
    auto blurmf = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5);
    auto gradmf = SkShaderMaskFilter::Make(make_shader(SkRect::MakeIWH(mask->width(),
                                                                       mask->height())));

    const sk_sp<SkMaskFilter> array[] = { nullptr , blurmf, gradmf };
    for (SkBlendMode mode : {SkBlendMode::kSrcOver, SkBlendMode::kSrcIn}) {
        canvas->save();
        for (sk_sp<SkMaskFilter> mf : array) {
            draw_masked_image(canvas, image.get(), 10, 10, mask.get(), mf, mode);
            canvas->translate(image->width() + 20.f, 0);
        }
        canvas->restore();
        canvas->translate(0, image->height() + 20.f);
    }
    return skiagm::DrawResult::kOk;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static sk_sp<SkMaskFilter> make_path_mf(const SkPath& path, unsigned alpha) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setAlpha(alpha);

    SkPictureRecorder recorder;
    recorder.beginRecording(1000, 1000)->drawPath(path, paint);
    auto shader = recorder.finishRecordingAsPicture()->makeShader(SkTileMode::kClamp,
                                                                  SkTileMode::kClamp);
    return SkShaderMaskFilter::Make(shader);
}

typedef void (*MakePathsProc)(const SkRect&, SkPath*, SkPath*);

const char* gCoverageName[] = {
    "union", "sect", "diff", "rev-diff", "xor"
};

DEF_SIMPLE_GM(combinemaskfilter, canvas, 560, 510) {
    const SkRect r = { 0, 0, 100, 100 };

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    SkFont font;
    font.setSize(20);

    const SkRect r2 = r.makeOutset(1.5f, 1.5f);
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);

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
        SkTextUtils::DrawString(canvas, gCoverageName[i], r.width()*0.5f, -10, font, SkPaint(),
                                       SkTextUtils::kCenter_Align);

        SkCoverageMode cmode = static_cast<SkCoverageMode>(i);
        canvas->save();
        // esp. on gpu side, its valuable to exercise modes that do and do-not convolve coverage
        // with alpha. SrcOver and SrcIn have these properties, but also happen to "look" the same
        // for this test.
        const SkBlendMode bmodes[] = { SkBlendMode::kSrcOver, SkBlendMode::kSrcIn };
        SkASSERT( SkBlendMode_SupportsCoverageAsAlpha(bmodes[0]));  // test as-alpha
        SkASSERT(!SkBlendMode_SupportsCoverageAsAlpha(bmodes[1]));  // test not-as-alpha
        for (auto bmode : bmodes) {
            paint.setBlendMode(bmode);
            for (int j = 0; j < 2; ++j) {
                paint.setMaskFilter(SkMaskFilter::MakeCombine(mfA[j], mfB[j], cmode));
                canvas->drawRect(r2, strokePaint);
                canvas->drawRect(r, paint);
                canvas->translate(0, r.height() + 10);
            }
            canvas->translate(0, 40);
        }
        canvas->restore();
        canvas->translate(r.width() + 10, 0);
    }
    canvas->restore();
}
