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
#include "tools/timer/Timer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#endif

SkRandom gRand;

static void DrawTheText(SkCanvas* canvas, const char text[], size_t length, SkScalar x, SkScalar y,
                        const SkFont& font, const SkPaint& paint) {
    SkFont f(font);
    f.setSubpixel(true);
    canvas->drawSimpleText(text, length, kUTF8_SkTextEncoding, x, y, f, paint);
}

// This sample demonstrates the cache behavior of bitmap vs. distance field text
// It renders variously sized text with an animated scale and rotation.
// Specifically one should:
//   use 'D' to toggle between bitmap and distance field fonts
//   use '2' to toggle between scaling the image by 2x
//            -- this feature boosts the rendering out of the small point-size
//               SDF-text special case (which falls back to bitmap fonts for small points)

class AnimatedTextView : public Sample {
public:
    AnimatedTextView() : fScale(1.0f), fScaleInc(0.1f), fRotation(0.0f), fSizeScale(1) {
        fCurrentTime = 0;
        fTimer.start();
        memset(fTimes, 0, sizeof(fTimes));
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "AnimatedText");
            return true;
        }

        SkUnichar uni;
        if (Sample::CharQ(*evt, &uni)) {
            if ('2' == uni) {
                if (fSizeScale == 2) {
                    fSizeScale = 1;
                } else {
                    fSizeScale = 2;
                }
                return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkFont font(SkTypeface::MakeFromFile("/skimages/samplefont.ttf"));

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setFilterQuality(kMedium_SkFilterQuality);

        SkString outString("fps: ");
        fTimer.end();

        // TODO: generalize this timing code in utils
        fTimes[fCurrentTime] = (float)(fTimer.fWall);
        fCurrentTime = (fCurrentTime + 1) & 0x1f;

        float meanTime = 0.0f;
        for (int i = 0; i < 32; ++i) {
            meanTime += fTimes[i];
        }
        meanTime /= 32.f;
        SkScalar fps = 1000.f / meanTime;
        outString.appendScalar(fps);
        outString.append(" ms: ");
        outString.appendScalar(meanTime);

        SkString modeString("Text scale: ");
        modeString.appendU32(fSizeScale);
        modeString.append("x");

        fTimer.start();

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
//        canvas->drawString(outString, 512.f, 540.f, paint);
        canvas->drawString(modeString, 768.f, 540.f, font, paint);
    }

    bool onAnimate(const AnimTimer& timer) override {
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

private:
    float fScale;
    float fScaleInc;
    float fRotation;
    int   fSizeScale;

    WallTimer   fTimer;
    float       fTimes[32];
    int         fCurrentTime;


    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new AnimatedTextView(); )
