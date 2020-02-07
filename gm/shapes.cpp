/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTArray.h"
#include "include/utils/SkRandom.h"

namespace skiagm {

/*
 * This is the base class for two GMs that cover various corner cases with primitive Skia shapes
 * (zero radius, near-zero radius, inner shape overlap, etc.) It uses an xfermode of darken to help
 * double-blended and/or dropped pixels stand out.
 */
class ShapesGM : public GM {
protected:
    ShapesGM(const char* name, bool antialias) : fName(name), fAntialias(antialias) {
        if (!antialias) {
            fName.append("_bw");
        }
    }

    SkString onShortName() override final { return fName; }
    SkISize onISize() override { return SkISize::Make(500, 500); }

    void onOnceBeforeDraw() override {
        fShapes.push_back().setOval(SkRect::MakeXYWH(-5, 25, 200, 100));
        fRotations.push_back(21);

        fShapes.push_back().setRect(SkRect::MakeXYWH(95, 75, 125, 100));
        fRotations.push_back(94);

        fShapes.push_back().setRectXY(SkRect::MakeXYWH(0, 75, 150, 100), 1e-5f, 1e-5f);
        fRotations.push_back(132);

        fShapes.push_back().setRectXY(SkRect::MakeXYWH(15, -20, 100, 100), 20, 15);
        fRotations.push_back(282);

        fSimpleShapeCount = fShapes.count();

        fShapes.push_back().setNinePatch(SkRect::MakeXYWH(140, -50, 90, 110), 10, 5, 25, 35);
        fRotations.push_back(0);

        fShapes.push_back().setNinePatch(SkRect::MakeXYWH(160, -60, 60, 90), 10, 60, 50, 30);
        fRotations.push_back(-35);

        fShapes.push_back().setNinePatch(SkRect::MakeXYWH(220, -120, 60, 90), 1, 89, 59, 1);
        fRotations.push_back(65);

        SkVector radii[4] = {{4, 6}, {12, 8}, {24, 16}, {32, 48}};
        fShapes.push_back().setRectRadii(SkRect::MakeXYWH(150, -129, 80, 160), radii);
        fRotations.push_back(265);

        SkVector radii2[4] = {{0, 0}, {80, 60}, {0, 0}, {80, 60}};
        fShapes.push_back().setRectRadii(SkRect::MakeXYWH(180, -30, 80, 60), radii2);
        fRotations.push_back(295);

        fPaint.setAntiAlias(fAntialias);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorWHITE);

        canvas->save();
        canvas->translate(canvas->imageInfo().width() / 2.f, canvas->imageInfo().height() / 2.f);
        this->drawShapes(canvas);
        canvas->restore();
    }

    virtual void drawShapes(SkCanvas* canvas) const = 0;

protected:
    SkString             fName;
    bool                 fAntialias;
    SkPaint              fPaint;
    SkTArray<SkRRect>    fShapes;
    SkTArray<SkScalar>   fRotations;
    int                  fSimpleShapeCount;

private:
    typedef GM INHERITED;
};

class SimpleShapesGM : public ShapesGM {
public:
    SimpleShapesGM(bool antialias) : INHERITED("simpleshapes", antialias) {}

private:
    void drawShapes(SkCanvas* canvas) const override {
        SkRandom rand(2);
        for (int i = 0; i < fShapes.count(); i++) {
            SkPaint paint(fPaint);
            paint.setColor(rand.nextU() & ~0x808080);
            paint.setAlphaf(0.5f);  // Use alpha to detect double blends.
            const SkRRect& shape = fShapes[i];
            canvas->save();
            canvas->rotate(fRotations[i]);
            switch (shape.getType()) {
                case SkRRect::kRect_Type:
                    canvas->drawRect(shape.rect(), paint);
                    break;
                case SkRRect::kOval_Type:
                    canvas->drawOval(shape.rect(), paint);
                    break;
                default:
                    canvas->drawRRect(shape, paint);
                    break;
            }
            canvas->restore();
        }
    }

    typedef ShapesGM INHERITED;
};

class InnerShapesGM : public ShapesGM {
public:
    InnerShapesGM(bool antialias) : INHERITED("innershapes", antialias) {}

private:
    void drawShapes(SkCanvas* canvas) const override {
        SkRandom rand;
        for (int i = 0; i < fShapes.count(); i++) {
            const SkRRect& outer = fShapes[i];
            const SkRRect& inner = fShapes[(i * 7 + 11) % fSimpleShapeCount];
            float s = 0.95f * std::min(outer.rect().width() / inner.rect().width(),
                                     outer.rect().height() / inner.rect().height());
            SkMatrix innerXform;
            float dx = (rand.nextF() - 0.5f) * (outer.rect().width() - s * inner.rect().width());
            float dy = (rand.nextF() - 0.5f) * (outer.rect().height() - s * inner.rect().height());
            innerXform.setTranslate(outer.rect().centerX() + dx, outer.rect().centerY() + dy);
            if (s < 1) {
                innerXform.preScale(s, s);
            }
            innerXform.preTranslate(-inner.rect().centerX(), -inner.rect().centerY());
            SkRRect xformedInner;
            inner.transform(innerXform, &xformedInner);
            SkPaint paint(fPaint);
            paint.setColor(rand.nextU() & ~0x808080);
            paint.setAlphaf(0.5f);  // Use alpha to detect double blends.
            canvas->save();
            canvas->rotate(fRotations[i]);
            canvas->drawDRRect(outer, xformedInner, paint);
            canvas->restore();
        }
    }

    typedef ShapesGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new SimpleShapesGM(true); )
DEF_GM( return new SimpleShapesGM(false); )
DEF_GM( return new InnerShapesGM(true); )
DEF_GM( return new InnerShapesGM(false); )

}
