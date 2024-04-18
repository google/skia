/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkFloatBits.h"

using namespace skia_private;

class ConicPathsGM : public skiagm::GM {
protected:
    SkString getName() const override { return SkString("conicpaths"); }

    SkISize getISize() override { return SkISize::Make(920, 960); }

    template <typename Proc> void append_path(Proc proc) {
        SkPathBuilder b;
        proc(&b);
        fPaths.push_back(b.detach());
    }

    void onOnceBeforeDraw() override {
        this->append_path([](SkPathBuilder* conicCircle) {
            const SkScalar w = SkScalarSqrt(2)/2;
            conicCircle->moveTo(0, 0);
            conicCircle->conicTo(0, 50, 50, 50, w);
            conicCircle->rConicTo(50, 0, 50, -50, w);
            conicCircle->rConicTo(0, -50, -50, -50, w);
            conicCircle->rConicTo(-50, 0, -50, 50, w);
        });

        this->append_path([](SkPathBuilder* hyperbola) {
            hyperbola->moveTo(0, 0);
            hyperbola->conicTo(0, 100, 100, 100, 2);
        });

        this->append_path([](SkPathBuilder* thinHyperbola) {
            thinHyperbola->moveTo(0, 0);
            thinHyperbola->conicTo(100, 100, 5, 0, 2);
        });

        this->append_path([](SkPathBuilder* veryThinHyperbola) {
            veryThinHyperbola->moveTo(0, 0);
            veryThinHyperbola->conicTo(100, 100, 1, 0, 2);
        });

        this->append_path([](SkPathBuilder* closedHyperbola) {
            closedHyperbola->moveTo(0, 0);
            closedHyperbola->conicTo(100, 100, 0, 0, 2);
        });

        this->append_path([](SkPathBuilder* nearParabola) {
            // using 1 as weight defaults to using quadTo
            nearParabola->moveTo(0, 0);
            nearParabola->conicTo(0, 100, 100, 100, 0.999f);
        });

        this->append_path([](SkPathBuilder* thinEllipse) {
            thinEllipse->moveTo(0, 0);
            thinEllipse->conicTo(100, 100, 5, 0, SK_ScalarHalf);
        });

        this->append_path([](SkPathBuilder* veryThinEllipse) {
            veryThinEllipse->moveTo(0, 0);
            veryThinEllipse->conicTo(100, 100, 1, 0, SK_ScalarHalf);
        });

        this->append_path([](SkPathBuilder* closedEllipse) {
            closedEllipse->moveTo(0,  0);
            closedEllipse->conicTo(100, 100, 0, 0, SK_ScalarHalf);
        });

        {
            SkPathBuilder b;
            const SkScalar w = SkScalarSqrt(2)/2;
            b.moveTo(2.1e+11f, -1.05e+11f);
            b.conicTo(2.1e+11f, 0, 1.05e+11f, 0, w);
            b.conicTo(0, 0, 0, -1.05e+11f, w);
            b.conicTo(0, -2.1e+11f, 1.05e+11f, -2.1e+11f, w);
            b.conicTo(2.1e+11f, -2.1e+11f, 2.1e+11f, -1.05e+11f, w);
            fGiantCircle = b.detach();
        }
    }

    void drawGiantCircle(SkCanvas* canvas) {
        SkPaint paint;
        canvas->drawPath(fGiantCircle, paint);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkAlpha kAlphaValue[] = { 0xFF, 0x40 };

        const SkScalar margin = 15;
        canvas->translate(margin, margin);

        SkPaint paint;
        for (int p = 0; p < fPaths.size(); ++p) {
            canvas->save();
            for (size_t a = 0; a < std::size(kAlphaValue); ++a) {
                paint.setARGB(kAlphaValue[a], 0, 0, 0);
                for (int aa = 0; aa < 2; ++aa) {
                    paint.setAntiAlias(SkToBool(aa));
                    for (int fh = 0; fh < 2; ++fh) {
                        paint.setStroke(fh != 0);

                        const SkRect& bounds = fPaths[p].getBounds();
                        canvas->save();
                        canvas->translate(-bounds.fLeft, -bounds.fTop);
                        canvas->drawPath(fPaths[p], paint);
                        canvas->restore();

                        canvas->translate(110, 0);
                    }
                }
            }
            canvas->restore();
            canvas->translate(0, 110);
        }
        canvas->restore();

        this->drawGiantCircle(canvas);
    }

private:
    TArray<SkPath> fPaths;
    SkPath           fGiantCircle;
    using INHERITED = skiagm::GM;
};
DEF_GM(return new ConicPathsGM;)

//////////////////////////////////////////////////////////////////////////////

/* arc should be on top of circle */
DEF_SIMPLE_GM(arccirclegap, canvas, 250, 250) {
    canvas->translate(50, 100);
    SkPoint c = { 1052.5390625f, 506.8760978034711f };
    SkScalar radius = 1096.702150363923f;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStroke(true);
    canvas->drawCircle(c, radius, paint);
    SkPath path = SkPathBuilder().moveTo(288.88884710654133f, -280.26680862609f)
                                 .arcTo({0, 0}, {-39.00216443306411f, 400.6058925796476f}, radius)
                                 .detach();
    paint.setColor(0xff007f00);
    canvas->drawPath(path, paint);
}

/* circle should be antialiased */
DEF_SIMPLE_GM(largecircle, canvas, 250, 250) {
    canvas->translate(50, 100);
    SkPoint c = { 1052.5390625f, 506.8760978034711f };
    SkScalar radius = 1096.702150363923f;
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStroke(true);
    canvas->drawCircle(c, radius, paint);
}

/* ovals should not be blurry */
DEF_SIMPLE_GM(largeovals, canvas, 250, 250) {
    // Test EllipseOp
    SkRect r = SkRect::MakeXYWH(-520, -520, 5000, 4000);
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStroke(true);
    paint.setStrokeWidth(100);
    canvas->drawOval(r, paint);
    r.offset(-15, -15);
    paint.setColor(SK_ColorDKGRAY);
    // we use stroke and fill to avoid falling into the SimpleFill path
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(1);
    canvas->drawOval(r, paint);

    // Test DIEllipseOp
    canvas->rotate(1.0f);
    r.offset(55, 55);
    paint.setColor(SK_ColorGRAY);
    paint.setStroke(true);
    paint.setStrokeWidth(100);
    canvas->drawOval(r, paint);
    r.offset(-15, -15);
    paint.setColor(SK_ColorLTGRAY);
    paint.setStyle(SkPaint::kStrokeAndFill_Style);
    paint.setStrokeWidth(1);
    canvas->drawOval(r, paint);
}

DEF_SIMPLE_GM(crbug_640176, canvas, 250, 250) {
    SkPathBuilder path;
    path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
    path.lineTo(SkBits2Float(0x42cfd89a), SkBits2Float(0xc2700000));  // 103.923f, -60
    path.lineTo(SkBits2Float(0x42cfd899), SkBits2Float(0xc2700006));  // 103.923f, -60
    path.conicTo(SkBits2Float(0x42f00000), SkBits2Float(0xc2009d9c),
            SkBits2Float(0x42f00001), SkBits2Float(0x00000000),
            SkBits2Float(0x3f7746ea));  // 120, -32.1539f, 120, 0, 0.965926f

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas->translate(125, 125);
    canvas->drawPath(path.detach(), paint);
}
