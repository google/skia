/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPictureRecorder.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/utils/SkTextUtils.h"
#include "src/core/SkBlendModePriv.h"
#include "tools/ToolUtils.h"

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
        mf = SkMaskFilter::MakeCompose(outer->makeWithMatrix(matrix), mf);
    }
    paint.setMaskFilter(mf);
    paint.setAntiAlias(true);
    paint.setBlendMode(mode);
    canvas->drawImage(image, x, y, &paint);
}

#include "include/effects/SkGradientShader.h"
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

#include "tools/Resources.h"
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

#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"

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

#include "include/core/SkMaskFilter.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkBlurImageFilter.h"
static sk_sp<SkImage> make_circle_image(SkCanvas* canvas, SkScalar radius, int margin) {
    const int n = SkScalarCeilToInt(radius) * 2 + margin * 2;
    auto      surf = ToolUtils::makeSurface(canvas, SkImageInfo::MakeN32Premul(n, n));
    SkPaint paint;
    paint.setAntiAlias(true);
    surf->getCanvas()->drawCircle(n * 0.5f, n * 0.5f, radius, paint);
    return surf->makeImageSnapshot();
}

DEF_SIMPLE_GM(savelayer_maskfilter, canvas, 450, 675) {
    auto layerImage = GetResourceAsImage("images/mandrill_128.png");
    auto maskImage = make_circle_image(canvas, 50, 1);
    SkRect r = SkRect::MakeWH(102, 102);

    SkPaint overlayPaint;
    overlayPaint.setStyle(SkPaint::kStroke_Style);

    // test that the maskfilter sees these changes to the ctm
    canvas->translate(10, 10);
    canvas->scale(2, 2);

    sk_sp<SkMaskFilter> mfs[] = {
        SkShaderMaskFilter::Make(maskImage->makeShader()),
        SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 3.5f),
        nullptr,
    };
    mfs[2] = SkMaskFilter::MakeCompose(mfs[1], mfs[0]);

    // Important that we test with and without an imagefilter attached to the layer,
    // as cpu and gpu backends treat these differently (w/ or w/o a SkSpecialImage)
    const sk_sp<SkImageFilter> imfs[] = {nullptr, SkBlurImageFilter::Make(3.5f, 3.5f, nullptr)};

    for (auto& mf : mfs) {
        SkPaint layerPaint;
        layerPaint.setMaskFilter(mf);
        canvas->save();
        for (auto& imf : imfs) {
            layerPaint.setImageFilter(imf);

            canvas->saveLayer(&r, &layerPaint);
            canvas->drawImage(layerImage, 0, 0, nullptr);
            canvas->restore();

            // now draw the (approximage) expected bounds of the mask
            canvas->drawRect(r.makeOutset(1, 1), overlayPaint);

            canvas->translate(r.width() + 10, 0);
        }
        canvas->restore();
        canvas->translate(0, r.height() + 10);
    }
}

static void draw_mask(SkCanvas* canvas) {
    SkPaint p;
    p.setAntiAlias(true);
    canvas->drawOval(SkRect::Make(canvas->imageInfo().bounds()), p);
}

DEF_SIMPLE_GM(shadermaskfilter_localmatrix, canvas, 1500, 1000) {
    static constexpr SkScalar kSize = 100;

    using ShaderMakerT = sk_sp<SkShader>(*)(SkCanvas*, const SkMatrix& lm);
    static const ShaderMakerT gShaderMakers[] = {
            [](SkCanvas* canvas, const SkMatrix& lm) -> sk_sp<SkShader> {
                auto surface =
                        ToolUtils::makeSurface(canvas, SkImageInfo::MakeN32Premul(kSize, kSize));
                draw_mask(surface->getCanvas());
                return surface->makeImageSnapshot()->makeShader(
                        SkTileMode::kClamp, SkTileMode::kClamp, &lm);
            },
            [](SkCanvas*, const SkMatrix& lm) -> sk_sp<SkShader> {
                SkPictureRecorder recorder;
                draw_mask(recorder.beginRecording(kSize, kSize));
                return recorder.finishRecordingAsPicture()->makeShader(
                                                   SkTileMode::kClamp,
                                                   SkTileMode::kClamp,
                                                   &lm,
                                                   nullptr);
            },
    };

    struct Config {
        SkMatrix fCanvasMatrix,
                 fMaskMatrix,
                 fShaderMatrix;
    } gConfigs[] = {
        { SkMatrix::I(), SkMatrix::MakeScale(2, 2), SkMatrix::MakeTrans(10, 10) },
        { SkMatrix::MakeScale(2, 2), SkMatrix::I(), SkMatrix::MakeTrans(10, 10) },
        { SkMatrix::MakeScale(2, 2), SkMatrix::MakeTrans(10, 10), SkMatrix::I() },
        { SkMatrix::Concat(SkMatrix::MakeScale(2, 2), SkMatrix::MakeTrans(10, 10)),
          SkMatrix::I(), SkMatrix::I() },
        { SkMatrix::I(),
          SkMatrix::Concat(SkMatrix::MakeScale(2, 2), SkMatrix::MakeTrans(10, 10)),
          SkMatrix::I() },
        { SkMatrix::I(), SkMatrix::I(),
          SkMatrix::Concat(SkMatrix::MakeScale(2, 2), SkMatrix::MakeTrans(10, 10)) },
    };

    using DrawerT = void(*)(SkCanvas*, const SkRect&, const SkPaint&);
    static const DrawerT gDrawers[] = {
        [](SkCanvas* canvas, const SkRect& dest, const SkPaint& mask) {
            canvas->drawRect(dest, mask);
        },
        [](SkCanvas* canvas, const SkRect& dest, const SkPaint& mask) {
            canvas->saveLayer(&dest, &mask);
            SkPaint p = mask;
            p.setMaskFilter(nullptr);
            canvas->drawPaint(p);
            canvas->restore();
        },
    };

    SkPaint paint, rectPaint;
    paint.setColor(0xff00ff00);
    rectPaint.setStyle(SkPaint::kStroke_Style);
    rectPaint.setColor(0xffff0000);

    for (const auto& sm : gShaderMakers) {
        for (const auto& drawer : gDrawers) {
            {
                SkAutoCanvasRestore acr(canvas, true);
                for (const auto& cfg : gConfigs) {
                    paint.setMaskFilter(SkShaderMaskFilter::Make(sm(canvas, cfg.fShaderMatrix))
                                        ->makeWithMatrix(cfg.fMaskMatrix));
                    auto dest = SkRect::MakeWH(kSize, kSize);
                    SkMatrix::Concat(cfg.fMaskMatrix, cfg.fShaderMatrix).mapRect(&dest);

                    {
                        SkAutoCanvasRestore acr(canvas, true);
                        canvas->concat(cfg.fCanvasMatrix);
                        drawer(canvas, dest, paint);
                        canvas->drawRect(dest, rectPaint);
                    }

                    canvas->translate(kSize * 2.5f, 0);
                }
            }
            canvas->translate(0, kSize * 2.5f);
        }

    }
}
