/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AnimTimer.h"
#include "Resources.h"
#include "Sample.h"
#include "SkCanvas.h"
#include "SkFont.h"
#include "SkRSXform.h"
#include "SkSurface.h"
#include "Timer.h"

#include <stdio.h>

static const int kGrid = 100;
static const int kWidth = 960;
static const int kHeight = 640;

typedef void (*DrawAtlasProc)(SkCanvas*, SkImage*, const SkRSXform[], const SkRect[],
const SkColor[], int, const SkRect*, const SkPaint*);

static void draw_atlas(SkCanvas* canvas, SkImage* atlas, const SkRSXform xform[],
                       const SkRect tex[], const SkColor colors[], int count, const SkRect* cull,
                       const SkPaint* paint) {
    canvas->drawAtlas(atlas, xform, tex, colors, count, SkBlendMode::kModulate, cull, paint);
}

static void draw_atlas_sim(SkCanvas* canvas, SkImage* atlas, const SkRSXform xform[],
                           const SkRect tex[], const SkColor colors[], int count, const SkRect* cull,
                           const SkPaint* paint) {
    for (int i = 0; i < count; ++i) {
        SkMatrix matrix;
        matrix.setRSXform(xform[i]);

        canvas->save();
        canvas->concat(matrix);
        canvas->drawImageRect(atlas, tex[i], tex[i].makeOffset(-tex[i].x(), -tex[i].y()), paint,
                              SkCanvas::kFast_SrcRectConstraint);
        canvas->restore();
    }
}


class DrawShipView : public Sample {
public:
    DrawShipView(const char name[], DrawAtlasProc proc) : fName(name), fProc(proc) {
        fAtlas = GetResourceAsImage("images/ship.png");
        if (!fAtlas) {
            SkDebugf("\nCould not decode file ship.png. Falling back to penguin mode.\n");
            fAtlas = GetResourceAsImage("images/baby_tux.png");
            if (!fAtlas) {
                SkDebugf("\nCould not decode file baby_tux.png. Did you forget"
                         " to set the resourcePath?\n");
                return;
            }
        }

        SkScalar anchorX = fAtlas->width()*0.5f;
        SkScalar anchorY = fAtlas->height()*0.5f;
        int currIndex = 0;
        for (int x = 0; x < kGrid; x++) {
            for (int y = 0; y < kGrid; y++) {
                float xPos = (x / (kGrid - 1.0f)) * kWidth;
                float yPos = (y / (kGrid - 1.0f)) * kWidth;

                fTex[currIndex] = SkRect::MakeLTRB(0.0f, 0.0f,
                                                   SkIntToScalar(fAtlas->width()),
                                                   SkIntToScalar(fAtlas->height()));
                fXform[currIndex] = SkRSXform::MakeFromRadians(0.1f, SK_ScalarPI*0.5f,
                                                               xPos, yPos, anchorX, anchorY);
                currIndex++;
            }
        }
        fTex[currIndex] = SkRect::MakeLTRB(0.0f, 0.0f,
                                           SkIntToScalar(fAtlas->width()),
                                           SkIntToScalar(fAtlas->height()));
        fXform[currIndex] = SkRSXform::MakeFromRadians(0.5f, SK_ScalarPI*0.5f,
                                                       kWidth*0.5f, kHeight*0.5f, anchorX, anchorY);

        fCurrentTime = 0;
        fTimer.start();
    }

    ~DrawShipView() override {}

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, fName);
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        const float kCosDiff = 0.99984769515f;
        const float kSinDiff = 0.01745240643f;

        if (!fAtlas) {
            return;
        }

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setColor(SK_ColorWHITE);

        SkFont font;
        font.setSize(15.0f);

        fTimer.end();

        fTimes[fCurrentTime] = (float)(fTimer.fWall);
        fCurrentTime = (fCurrentTime + 1) & 0x1f;

        float meanTime = 0.0f;
        for (int i = 0; i < 32; ++i) {
            meanTime += fTimes[i];
        }
        meanTime /= 32.f;
        SkString outString("fps: ");
        SkScalar fps = 1000.f/meanTime;
        outString.appendScalar(fps);
        outString.append(" ms: ");
        outString.appendScalar(meanTime);

        fTimer.start();

        SkScalar anchorX = fAtlas->width()*0.5f;
        SkScalar anchorY = fAtlas->height()*0.5f;
        for (int i = 0; i < kGrid*kGrid+1; ++i) {
            SkScalar c = fXform[i].fSCos;
            SkScalar s = fXform[i].fSSin;

            SkScalar dx = c*anchorX - s*anchorY;
            SkScalar dy = s*anchorX + c*anchorY;

            fXform[i].fSCos = kCosDiff*c - kSinDiff*s;
            fXform[i].fSSin = kSinDiff*c + kCosDiff*s;

            dx -= fXform[i].fSCos*anchorX - fXform[i].fSSin*anchorY;
            dy -= fXform[i].fSSin*anchorX + fXform[i].fSCos*anchorY;
            fXform[i].fTx += dx;
            fXform[i].fTy += dy;
        }

        fProc(canvas, fAtlas.get(), fXform, fTex, nullptr, kGrid*kGrid+1, nullptr, &paint);
        paint.setColor(SK_ColorBLACK);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, 200, 24), paint);
        paint.setColor(SK_ColorWHITE);
        canvas->drawString(outString, 5, 15, font, paint);
    }

#if 0
    // TODO: switch over to use this for our animation
    bool onAnimate(const AnimTimer& timer) override {
        SkScalar angle = SkDoubleToScalar(fmod(timer.secs() * 360 / 24, 360));
        fAnimatingDrawable->setSweep(angle);
        return true;
    }
#endif

private:
    const char*         fName;
    DrawAtlasProc       fProc;

    sk_sp<SkImage> fAtlas;
    SkRSXform   fXform[kGrid*kGrid+1];
    SkRect      fTex[kGrid*kGrid+1];
    WallTimer   fTimer;
    float       fTimes[32];
    int         fCurrentTime;


    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new DrawShipView("DrawShip", draw_atlas); )
DEF_SAMPLE( return new DrawShipView("DrawShipSim", draw_atlas_sim); )
