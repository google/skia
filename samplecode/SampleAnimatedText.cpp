/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkRandom.h"
#include "SkSystemEventTypes.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkXfermode.h"
#include "Timer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "SkGpuDevice.h"
#endif

SkRandom gRand;

static void DrawTheText(SkCanvas* canvas, const char text[], size_t length, SkScalar x, SkScalar y,
                        const SkPaint& paint) {
    SkPaint p(paint);

    p.setSubpixelText(true);
    canvas->drawText(text, length, x, y, p);
}

// This sample demonstrates the cache behavior of bitmap vs. distance field text
// It renders variously sized text with an animated scale and rotation.
// Specifically one should:
//   use 'D' to toggle between bitmap and distance field fonts
//   use '2' to toggle between scaling the image by 2x
//            -- this feature boosts the rendering out of the small point-size
//               SDF-text special case (which falls back to bitmap fonts for small points)

class AnimatedTextView : public SampleView {
public:
    AnimatedTextView() : fScale(1.0f), fScaleInc(0.1f), fRotation(0.0f), fSizeScale(1) {
        fCurrentTime = 0;
        fTimer.start();
        memset(fTimes, 0, sizeof(fTimes));
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AnimatedText");
            return true;
        }

        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
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
        SkPaint paint;
        paint.setTypeface(SkTypeface::MakeFromFile("/skimages/samplefont.ttf"));
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
        SkBaseDevice* device = canvas->getDevice_just_for_deprecated_compatibility_testing();
        GrContext* grContext = canvas->getGrContext();
        if (grContext) {
            GrTexture* tex = grContext->getFontAtlasTexture(GrMaskFormat::kA8_GrMaskFormat);
            reinterpret_cast<SkGpuDevice*>(device)->drawTexture(tex,
                                                       SkRect::MakeXYWH(512, 10, 512, 512), paint);
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
            paint.setTextSize(SkIntToScalar(i*fSizeScale));
            y += paint.getFontSpacing();
            DrawTheText(canvas, text, length, SkIntToScalar(110), y, paint);
        }
        canvas->restore();

        paint.setTextSize(16);
//        canvas->drawText(outString.c_str(), outString.size(), 512.f, 540.f, paint);
        canvas->drawText(modeString.c_str(), modeString.size(), 768.f, 540.f, paint);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
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


    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AnimatedTextView; }
static SkViewRegister reg(MyFactory);
