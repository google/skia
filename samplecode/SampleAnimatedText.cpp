/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkImage.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#endif

SkRandom gRand;

static void DrawTheText(SkCanvas* canvas, const char text[], size_t length, SkScalar x, SkScalar y,
                        const SkFont& font, const SkPaint& paint) {
    SkFont f(font);
    f.setSubpixel(true);
    canvas->drawSimpleText(text, length, SkTextEncoding::kUTF8, x, y, f, paint);
}

// This sample demonstrates the cache behavior of bitmap vs. distance field text
// It renders variously sized text with an animated scale and rotation.
// Specifically one should:
//   use 'D' to toggle between bitmap and distance field fonts
//   use '2' to toggle between scaling the image by 2x
//            -- this feature boosts the rendering out of the small point-size
//               SDF-text special case (which falls back to bitmap fonts for small points)

class AnimatedTextView : public Sample {
    float fScale = 1;
    float fScaleInc = 0.1f;
    float fRotation = 0;
    int   fSizeScale = 1;

    SkString name() override { return SkString("AnimatedText"); }

    bool onChar(SkUnichar uni) override {
            if ('2' == uni) {
                if (fSizeScale == 2) {
                    fSizeScale = 1;
                } else {
                    fSizeScale = 2;
                }
                return true;
            }
            return false;
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkFont font(SkTypeface::MakeFromFile("/skimages/samplefont.ttf"));

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterQuality(kMedium_SkFilterQuality);

        canvas->save();

#if SK_SUPPORT_GPU
        GrContext* grContext = canvas->getGrContext();
        if (grContext) {
            sk_sp<SkImage> image = grContext->priv().testingOnly_getFontAtlasImage(
                                                                GrMaskFormat::kA8_GrMaskFormat);
            canvas->drawImageRect(image,
                                  SkRect::MakeXYWH(512.0f, 10.0f, 512.0f, 512.0f), &paint);
        }
#endif
        canvas->translate(180, 180);
        canvas->rotate(fRotation);
        canvas->scale(fScale, fScale);
        canvas->translate(-180, -180);

        const char* text = "Hamburgefons";
        size_t length = strlen(text);

        SkScalar y = SkIntToScalar(0);
        for (int i = 12; i <= 26; i++) {
            font.setSize(SkIntToScalar(i*fSizeScale));
            y += font.getSpacing();
            DrawTheText(canvas, text, length, SkIntToScalar(110), y, font, paint);
        }
        canvas->restore();

        font.setSize(16);
    }

    bool onAnimate(double nanos) override {
        // TODO: use nanos
        // We add noise to the scale and rotation animations to
        // keep the font atlas from falling into a steady state
        fRotation += (1.0f + gRand.nextRangeF(-0.1f, 0.1f));
        fScale += (fScaleInc + gRand.nextRangeF(-0.025f, 0.025f));
        if (fScale >= 2.0f) {
            fScaleInc = -0.1f;
        } else if (fScale <= 1.0f) {
            fScaleInc = 0.1f;
        }
        return true;
    }
};

DEF_SAMPLE( return new AnimatedTextView(); )
