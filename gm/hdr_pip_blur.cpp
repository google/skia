/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkImageFilters.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "tools/DecodeUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#endif



// Emulates rendering operations and color space conversions on Android running in HDR mode,
// mixing SDR background content, an HDR PiP "video", and an overall blur shade from a swipe.
class HDRPiPBlurGM : public skiagm::GM {
    // This GM is fragment bound, so when benchmarking, use a large texture size. Use a smaller
    // size for DM to reduce device load.
    static constexpr SkISize kFullSize{2560, 1440};
    static constexpr SkISize kNonBenchSize{640, 360};
public:
    HDRPiPBlurGM() {}

protected:
    SkString getName() const override { return SkString("hdr-pip-blur"); }

    SkISize getISize() override {
        if (this->getMode() == kBench_Mode) {
            return kFullSize;
        } else {
            return kNonBenchSize;
        }
    }

    bool runAsBench() const override { return true; }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        // When running in DM, all offscreen passes and the final rendering will be small.
        SkIRect screenBounds = SkIRect::MakeSize(this->getMode() == kGM_Mode ? kNonBenchSize
                                                                             : kFullSize);

        sk_sp<SkImage> input;
        {
            // The main surface is RGBA8 but with a wider gamut sRGB colorspace.
            sk_sp<SkSurface> content = canvas->makeSurface(
                    canvas->imageInfo().makeWH(screenBounds.width(), screenBounds.height())
                                       .makeColorType(kRGBA_8888_SkColorType)
                                       .makeColorSpace(
                                                SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020,
                                                                      SkNamedGamut::kDisplayP3)));
            if (!content) {
                // This occurs in DDL dm configs
                *errorMsg = "Could not create offscreen surface";
                return DrawResult::kSkip;
            }

            SkCanvas* c = content->getCanvas();
            c->clear(SkColors::kDkGray);

            SkMatrix toScreenBounds = SkMatrix::RectToRect(SkRect::Make(kFullSize),
                                                           SkRect::Make(screenBounds));
            c->concat(toScreenBounds);

            // Now render everything to `c` as if it were a kFullSize image.
            c->drawImageRect(fBackgroundImage.get(),
                             SkRect::Make(fBackgroundImage->bounds()),
                             SkRect::Make(kFullSize),
                             SkFilterMode::kLinear,
                             &fPaint,
                             SkCanvas::kFast_SrcRectConstraint);

            SkRRect pip = SkRRect::MakeRectXY(SkRect::MakeXYWH(1500.f, 700.f, 800.f, 600.f),
                                              20.f, 20.f);
            c->drawRRect(pip, fPaint);
            c->save();
                c->clipRRect(pip, true);
                c->drawImageRect(fPiPImage.get(),
                                 SkRect::MakeIWH(fPiPImage->width(), fPiPImage->height()),
                                 pip.rect(),
                                 SkFilterMode::kLinear,
                                 &fPaint,
                                 SkCanvas::kFast_SrcRectConstraint);
            c->restore();
            input = content->makeTemporaryImage();
        }

        canvas->save();

        if (this->getMode() == kSample_Mode) {
            // For viewer, the offscreen passes operate at full resolution, but we draw smaller to
            // fit into the window. This lets overall frame times match nanobench, but it looks like
            // what dm produces.
            SkMatrix toViewBounds = SkMatrix::RectToRect(SkRect::Make(screenBounds),
                                                         SkRect::Make(kNonBenchSize));
            canvas->concat(toViewBounds);
        }

        canvas->clipIRect(screenBounds);
        canvas->drawImage(input.get(), 0.f, 0.f, SkFilterMode::kLinear, &fPaint);

        // Explicitly blur the input image
        // NOTE: Skip calling MakeWithFilter and comment out the last `drawImageRect` call to
        // disable the shade blur.
        SkIRect outSubset;
        SkIPoint outOffset;
        sk_sp<SkImage> blur;
#if defined(SK_GRAPHITE)
        if (canvas->recorder()) {
            blur = SkImages::MakeWithFilter(canvas->recorder(),
                                            input,
                                            fShadeBlur.get(),
                                            screenBounds, screenBounds,
                                            &outSubset, &outOffset);
        } else
#endif
        if (canvas->recordingContext()) {
            blur = SkImages::MakeWithFilter(canvas->recordingContext(),
                                            input,
                                            fShadeBlur.get(),
                                            screenBounds, screenBounds,
                                            &outSubset, &outOffset);
        } else {
            blur = SkImages::MakeWithFilter(input,
                                            fShadeBlur.get(),
                                            screenBounds, screenBounds,
                                            &outSubset, &outOffset);
        }

        SkPaint fadedBlur;
        fadedBlur.setAlphaf(0.9f);
        canvas->drawImageRect(blur.get(),
                              SkRect::Make(outSubset),
                              SkRect::MakeXYWH(outOffset.x(), outOffset.y(),
                                               outSubset.width(), outSubset.height()),
                              SkFilterMode::kLinear,
                              &fadedBlur,
                              SkCanvas::kFast_SrcRectConstraint);

        canvas->restore();
        return DrawResult::kOk;
    }

    void onOnceBeforeDraw() override {
        fPaint.setColor4f({ 0.0f, 0.0f, 0.0f, 1.f });

        SkColorMatrix cm;
        cm.setSaturation(1.5f);
        fPaint.setColorFilter(SkColorFilters::Matrix(cm));

        // The background image is standard sRGB
        fBackgroundImage = ToolUtils::GetResourceAsImage("images/yellow_rose.png")
                                    ->makeRasterImage(nullptr)
                                    ->makeColorSpace(nullptr, SkColorSpace::MakeSRGB());
        // The pip will be PQ-ish HDR
        fPiPImage = ToolUtils::GetResourceAsImage("images/mandrill_512.png")
                             ->makeRasterImage(nullptr)
                             ->makeColorSpace(nullptr,
                                              SkColorSpace::MakeRGB(SkNamedTransferFn::kPQ,
                                                                    SkNamedGamut::kRec2020));

        const float sigma = 32.f;
        const bool scaleSigma = this->getMode() == kGM_Mode;
        const float sigmaX = sigma * (scaleSigma ? kNonBenchSize.width() : kFullSize.width()) /
                                     (float) kFullSize.width();
        const float sigmaY = sigma * (scaleSigma ? kNonBenchSize.height() : kFullSize.height()) /
                                     (float) kFullSize.height();
        fShadeBlur = SkImageFilters::Blur(sigmaX, sigmaY, SkTileMode::kClamp, nullptr);
    }

private:
    sk_sp<SkImage> fBackgroundImage;
    sk_sp<SkImage> fPiPImage;

    sk_sp<SkImageFilter> fShadeBlur;
    SkPaint fPaint;
};
DEF_GM( return new HDRPiPBlurGM; )
