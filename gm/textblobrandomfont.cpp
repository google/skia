/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Resources.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkStream.h"
#include "SkSurface.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "../src/fonts/SkRandomScalerContext.h"

#if SK_SUPPORT_GPU

#include "GrContext.h"

namespace skiagm {
class TextBlobRandomFont : public GM {
public:
    // This gm tests that textblobs can be translated and scaled with a font that returns random
    // but deterministic masks
    TextBlobRandomFont() { }

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        const char* text = "The quick brown fox jumps over the lazy dog.";

        // make textbloben
        SkPaint paint;
        paint.setTextSize(32);
        paint.setLCDRenderText(true);

        // Setup our random scaler context
        SkAutoTUnref<SkTypeface> orig(sk_tool_utils::create_portable_typeface("sans-serif",
                                                                              SkTypeface::kBold));
        if (NULL == orig) {
            orig.reset(SkTypeface::RefDefault());
        }
        SkAutoTUnref<SkTypeface> random(SkNEW_ARGS(SkRandomTypeface, (orig, paint, false)));
        paint.setTypeface(random);

        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);

        SkScalar yOffset = bounds.height();
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        // A8
        paint.setSubpixelText(false);
        paint.setLCDRenderText(false);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, yOffset - 32);

        // build
        fBlob.reset(builder.build());
    }

    SkString onShortName() override {
        return SkString("textblobrandomfont");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        // This GM exists to test a specific feature of the GPU backend.
        if (NULL == canvas->getGrContext()) {
            this->drawGpuOnlyMessage(canvas);
            return;
        }

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorWHITE));

        SkImageInfo info = SkImageInfo::MakeN32Premul(kWidth, kHeight);
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        SkAutoTUnref<SkSurface> surface(canvas->newSurface(info, &props));
        if (surface) {
            SkPaint paint;
            paint.setAntiAlias(true);

            SkCanvas* c = surface->getCanvas();

            int stride = SkScalarCeilToInt(fBlob->bounds().height() / 2) + 10;
            int yOffset = stride;
            for (int i = 0; i < 10; i++) {
                // fiddle the canvas to force regen of textblobs
                canvas->rotate(i % 2 ? 0.0f : -0.05f);
                canvas->drawTextBlob(fBlob, 10.0f, SkIntToScalar(yOffset), paint);
                yOffset += stride;

                // This will draw as black boxes
                c->drawTextBlob(fBlob, 10, SkIntToScalar(yOffset), paint);
                surface->draw(canvas, 0, 0, nullptr);

                // free gpu resources and verify
                yOffset += stride;
                canvas->getGrContext()->freeGpuResources();
                canvas->drawTextBlob(fBlob, 10, SkIntToScalar(yOffset), paint);

                yOffset += stride;
            }

        } else {
            const char* text = "This test requires a surface";
            size_t len = strlen(text);
            SkPaint paint;
            canvas->drawText(text, len, 10, 100, paint);
        }
    }

private:
    SkAutoTUnref<const SkTextBlob> fBlob;

    static const int kWidth = 1000;
    static const int kHeight = 1000;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(TextBlobRandomFont); )
}
#endif
