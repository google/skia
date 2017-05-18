/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

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
        sk_sp<SkTypeface> orig(sk_tool_utils::create_portable_typeface(
                                   "sans-serif", SkFontStyle::FromOldStyle(SkTypeface::kBold)));
        if (nullptr == orig) {
            orig = SkTypeface::MakeDefault();
        }
        paint.setTypeface(sk_make_sp<SkRandomTypeface>(orig, paint, false));

        SkRect bounds;
        paint.measureText(text, strlen(text), &bounds);
        sk_tool_utils::add_to_text_blob(&builder, text, paint, 0, 0);

        // A8
        const char* bigtext1 = "The quick brown fox";
        const char* bigtext2 = "jumps over the lazy dog.";
        paint.setTextSize(160);
        paint.setSubpixelText(false);
        paint.setLCDRenderText(false);
        paint.measureText(bigtext1, strlen(bigtext1), &bounds);
        SkScalar offset = bounds.height();
        sk_tool_utils::add_to_text_blob(&builder, bigtext1, paint, 0, offset);

        paint.measureText(bigtext2, strlen(bigtext2), &bounds);
        offset += bounds.height();
        sk_tool_utils::add_to_text_blob(&builder, bigtext2, paint, 0, offset);

        // color emoji
        sk_sp<SkTypeface> origEmoji = sk_tool_utils::emoji_typeface();
        const char* osName = sk_tool_utils::platform_os_name();
        // The mac emoji string will break us
        if (origEmoji && (!strcmp(osName, "Android") || !strcmp(osName, "Ubuntu"))) {
            const char* emojiText = sk_tool_utils::emoji_sample_text();
            paint.measureText(emojiText, strlen(emojiText), &bounds);
            offset += bounds.height();
            paint.setTypeface(sk_make_sp<SkRandomTypeface>(orig, paint, false));
            sk_tool_utils::add_to_text_blob(&builder, emojiText, paint, 0, offset);
        }

        // build
        fBlob = builder.make();
    }

    SkString onShortName() override {
        return SkString("textblobrandomfont");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {
        // This GM exists to test a specific feature of the GPU backend.
        if (nullptr == canvas->getGrContext()) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorWHITE));

        SkImageInfo info = SkImageInfo::Make(kWidth, kHeight, canvas->imageInfo().colorType(),
                                             kPremul_SkAlphaType,
                                             canvas->imageInfo().refColorSpace());
        SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
        auto surface(canvas->makeSurface(info, &props));
        if (surface) {
            SkPaint paint;
            paint.setAntiAlias(true);

            SkCanvas* c = surface->getCanvas();

            int stride = SkScalarCeilToInt(fBlob->bounds().height());
            int yOffset = stride / 8;
            for (int i = 0; i < 1; i++) {
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
    sk_sp<SkTextBlob> fBlob;

    static constexpr int kWidth = 2000;
    static constexpr int kHeight = 1600;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobRandomFont;)
}
#endif
