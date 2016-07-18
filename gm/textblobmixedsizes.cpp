/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

namespace skiagm {
class TextBlobMixedSizes : public GM {
public:
    // This gm tests that textblobs of mixed sizes with a large glyph will render properly
    TextBlobMixedSizes(bool useDFT) : fUseDFT(useDFT) {}

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // make textblob.  To stress distance fields, we choose sizes appropriately
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
        paint.setLCDRenderText(true);
        paint.setTypeface(MakeResourceAsTypeface("/fonts/HangingS.ttf"));

        const char* text = "Skia";

        // extra large
        paint.setTextSize(262);

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        // large
        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);
        SkScalar yOffset = bounds.height();
        paint.setTextSize(162);

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset);

        // Medium
        paint.measureText(text, strlen(text), &bounds);
        yOffset += bounds.height();
        paint.setTextSize(72);

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset);

        // Small
        paint.measureText(text, strlen(text), &bounds);
        yOffset += bounds.height();
        paint.setTextSize(32);

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset);

        // micro (will fall out of distance field text even if distance field text is enabled)
        paint.measureText(text, strlen(text), &bounds);
        yOffset += bounds.height();
        paint.setTextSize(14);

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset);

        // Zero size.
        paint.measureText(text, strlen(text), &bounds);
        yOffset += bounds.height();
        paint.setTextSize(0);

        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset);

        // build
        fBlob.reset(builder.build());
    }

    SkString onShortName() override {
        SkString name("textblobmixedsizes");
        if (fUseDFT) {
            name.appendf("_df");
        }
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* inputCanvas) override {
        SkCanvas* canvas = inputCanvas;
        sk_sp<SkSurface> surface;
        if (fUseDFT) {
#if SK_SUPPORT_GPU
            // Create a new Canvas to enable DFT
            GrContext* ctx = inputCanvas->getGrContext();
            SkISize size = onISize();
            sk_sp<SkColorSpace> colorSpace = sk_ref_sp(inputCanvas->imageInfo().colorSpace());
            SkImageInfo info = SkImageInfo::MakeN32(size.width(), size.height(),
                                                    kPremul_SkAlphaType, colorSpace);
            SkSurfaceProps canvasProps(SkSurfaceProps::kLegacyFontHost_InitType);
            uint32_t gammaCorrect = inputCanvas->getProps(&canvasProps)
                ? canvasProps.flags() & SkSurfaceProps::kGammaCorrect_Flag : 0;
            SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag | gammaCorrect,
                                 SkSurfaceProps::kLegacyFontHost_InitType);
            surface = SkSurface::MakeRenderTarget(ctx, SkBudgeted::kNo, info, 0, &props);
            canvas = surface.get() ? surface->getCanvas() : inputCanvas;
            // init our new canvas with the old canvas's matrix
            canvas->setMatrix(inputCanvas->getTotalMatrix());
#endif
        }
        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorWHITE));

        SkRect bounds = fBlob->bounds();

        static const int kPadX = SkScalarFloorToInt(bounds.width() / 3);
        static const int kPadY = SkScalarFloorToInt(bounds.height() / 3);

        int rowCount = 0;
        canvas->translate(SkIntToScalar(kPadX), SkIntToScalar(kPadY));
        canvas->save();
        SkRandom random;

        SkPaint paint;
        if (!fUseDFT) {
            paint.setColor(sk_tool_utils::color_to_565(SK_ColorWHITE));
        }
        paint.setAntiAlias(false);

        static const SkScalar kSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(8));

        // setup blur paint
        SkPaint blurPaint(paint);
        blurPaint.setColor(sk_tool_utils::color_to_565(SK_ColorBLACK));
        blurPaint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, kSigma));
        
        for (int i = 0; i < 4; i++) {
            canvas->save();
            switch (i % 2) {
                case 0:
                    canvas->rotate(random.nextF() * 45.f);
                    break;
                case 1:
                    canvas->rotate(-random.nextF() * 45.f);
                    break;
            }
            if (!fUseDFT) {
                canvas->drawTextBlob(fBlob, 0, 0, blurPaint);
            }
            canvas->drawTextBlob(fBlob, 0, 0, paint);
            canvas->restore();
            canvas->translate(bounds.width() + SK_Scalar1 * kPadX, 0);
            ++rowCount;
            if ((bounds.width() + 2 * kPadX) * rowCount > kWidth) {
                canvas->restore();
                canvas->translate(0, bounds.height() + SK_Scalar1 * kPadY);
                canvas->save();
                rowCount = 0;
            }
        }
        canvas->restore();

#if SK_SUPPORT_GPU
        // render offscreen buffer
        if (surface) {
            SkAutoCanvasRestore acr(inputCanvas, true);
            // since we prepended this matrix already, we blit using identity
            inputCanvas->resetMatrix();
            inputCanvas->drawImage(surface->makeImageSnapshot().get(), 0, 0, nullptr);
        }
#endif
    }

private:
    SkAutoTUnref<const SkTextBlob> fBlob;

    static const int kWidth = 2100;
    static const int kHeight = 1900;

    bool fUseDFT;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new TextBlobMixedSizes(false); )
#if SK_SUPPORT_GPU
DEF_GM( return new TextBlobMixedSizes(true); )
#endif
}
