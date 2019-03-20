/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "AnimTimer.h"
#include "Sample.h"
#include "SkCanvas.h"
#include "SkDrawable.h"
#include "SkPath.h"
#include "SkRSXform.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkTextUtils.h"

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

static sk_sp<SkImage> make_atlas(int atlasSize, int cellSize) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(atlasSize, atlasSize);
    auto surface(SkSurface::MakeRaster(info));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    SkRandom rand;

    const SkScalar half = cellSize * SK_ScalarHalf;
    const char* s = "01234567890!@#$%^&*=+<>?abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    SkFont font(nullptr, 28);

    int i = 0;
    for (int y = 0; y < atlasSize; y += cellSize) {
        for (int x = 0; x < atlasSize; x += cellSize) {
            paint.setColor(rand.nextU());
            paint.setAlpha(0xFF);
            int index = i % strlen(s);
            SkTextUtils::Draw(canvas, &s[index], 1, kUTF8_SkTextEncoding,
                              x + half, y + half + half/2, font, paint,
                              SkTextUtils::kCenter_Align);
            i += 1;
        }
    }
    return surface->makeImageSnapshot();
}

class DrawAtlasDrawable : public SkDrawable {
    enum {
        kMaxScale = 2,
        kCellSize = 32,
        kAtlasSize = 512,
    };

    struct Rec {
        SkPoint     fCenter;
        SkVector    fVelocity;
        SkScalar    fScale;
        SkScalar    fDScale;
        SkScalar    fRadian;
        SkScalar    fDRadian;
        SkScalar    fAlpha;
        SkScalar    fDAlpha;

        void advance(const SkRect& bounds) {
            fCenter += fVelocity;
            if (fCenter.fX > bounds.right()) {
                SkASSERT(fVelocity.fX > 0);
                fVelocity.fX = -fVelocity.fX;
            } else if (fCenter.fX < bounds.left()) {
                SkASSERT(fVelocity.fX < 0);
                fVelocity.fX = -fVelocity.fX;
            }
            if (fCenter.fY > bounds.bottom()) {
                if (fVelocity.fY > 0) {
                    fVelocity.fY = -fVelocity.fY;
                }
            } else if (fCenter.fY < bounds.top()) {
                if (fVelocity.fY < 0) {
                    fVelocity.fY = -fVelocity.fY;
                }
            }

            fScale += fDScale;
            if (fScale > 2 || fScale < SK_Scalar1/2) {
                fDScale = -fDScale;
            }

            fRadian += fDRadian;
            fRadian = SkScalarMod(fRadian, 2 * SK_ScalarPI);

            fAlpha += fDAlpha;
            if (fAlpha > 1) {
                fAlpha = 1;
                fDAlpha = -fDAlpha;
            } else if (fAlpha < 0) {
                fAlpha = 0;
                fDAlpha = -fDAlpha;
            }
        }

        SkRSXform asRSXform() const {
            return SkRSXform::MakeFromRadians(fScale, fRadian, fCenter.x(), fCenter.y(),
                                              SkScalarHalf(kCellSize), SkScalarHalf(kCellSize));
        }
    };

    DrawAtlasProc fProc;

    enum {
        N = 256,
    };

    sk_sp<SkImage> fAtlas;
    Rec         fRec[N];
    SkRect      fTex[N];
    SkRect      fBounds;
    bool        fUseColors;

public:
    DrawAtlasDrawable(DrawAtlasProc proc, const SkRect& r)
        : fProc(proc), fBounds(r), fUseColors(false)
    {
        SkRandom rand;
        fAtlas = make_atlas(kAtlasSize, kCellSize);
        const SkScalar kMaxSpeed = 5;
        const SkScalar cell = SkIntToScalar(kCellSize);
        int i = 0;
        for (int y = 0; y < kAtlasSize; y += kCellSize) {
            for (int x = 0; x < kAtlasSize; x += kCellSize) {
                const SkScalar sx = SkIntToScalar(x);
                const SkScalar sy = SkIntToScalar(y);
                fTex[i].setXYWH(sx, sy, cell, cell);

                fRec[i].fCenter.set(sx + cell/2, sy + 3*cell/4);
                fRec[i].fVelocity.fX = rand.nextSScalar1() * kMaxSpeed;
                fRec[i].fVelocity.fY = rand.nextSScalar1() * kMaxSpeed;
                fRec[i].fScale = 1;
                fRec[i].fDScale = rand.nextSScalar1() / 16;
                fRec[i].fRadian = 0;
                fRec[i].fDRadian = rand.nextSScalar1() / 8;
                fRec[i].fAlpha = rand.nextUScalar1();
                fRec[i].fDAlpha = rand.nextSScalar1() / 10;
                i += 1;
            }
        }
    }

    void toggleUseColors() {
        fUseColors = !fUseColors;
    }

protected:
    void onDraw(SkCanvas* canvas) override {
        SkRSXform xform[N];
        SkColor colors[N];

        for (int i = 0; i < N; ++i) {
            fRec[i].advance(fBounds);
            xform[i] = fRec[i].asRSXform();
            if (fUseColors) {
                colors[i] = SkColorSetARGB((int)(fRec[i].fAlpha * 0xFF), 0xFF, 0xFF, 0xFF);
            }
        }
        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);

        const SkRect cull = this->getBounds();
        const SkColor* colorsPtr = fUseColors ? colors : nullptr;
        fProc(canvas, fAtlas.get(), xform, fTex, colorsPtr, N, &cull, &paint);
    }

    SkRect onGetBounds() override {
        const SkScalar border = kMaxScale * kCellSize;
        SkRect r = fBounds;
        r.outset(border, border);
        return r;
    }

private:
    typedef SkDrawable INHERITED;
};

class DrawAtlasView : public Sample {
    const char* fName;
    DrawAtlasProc fProc;
    sk_sp<DrawAtlasDrawable> fDrawable;

public:
    DrawAtlasView(const char name[], DrawAtlasProc proc) : fName(name), fProc(proc) { }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, fName);
            return true;
        }
        SkUnichar uni;
        if (Sample::CharQ(*evt, &uni)) {
            switch (uni) {
                case 'C': fDrawable->toggleUseColors(); return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onOnceBeforeDraw() override {
        fDrawable = sk_make_sp<DrawAtlasDrawable>(fProc, SkRect::MakeWH(640, 480));
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawDrawable(fDrawable.get());
    }

    bool onAnimate(const AnimTimer&) override { return true; }
#if 0
    // TODO: switch over to use this for our animation
    bool onAnimate(const AnimTimer& timer) override {
        SkScalar angle = SkDoubleToScalar(fmod(timer.secs() * 360 / 24, 360));
        fAnimatingDrawable->setSweep(angle);
        return true;
    }
#endif

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new DrawAtlasView("DrawAtlas", draw_atlas); )
DEF_SAMPLE( return new DrawAtlasView("DrawAtlasSim", draw_atlas_sim); )
