/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkTArray.h"

namespace skiagm {

class HairlinesGM : public GM {
protected:


    SkString onShortName() override {
        return SkString("hairlines");
    }

    SkISize onISize() override { return SkISize::Make(1250, 1250); }

    void onOnceBeforeDraw() override {
        {
            SkPath* lineAnglesPath = &fPaths.push_back();
            enum {
                kNumAngles = 15,
                kRadius = 40,
            };
            for (int i = 0; i < kNumAngles; ++i) {
                SkScalar angle = SK_ScalarPI * SkIntToScalar(i) / kNumAngles;
                SkScalar x = kRadius * SkScalarCos(angle);
                SkScalar y = kRadius * SkScalarSin(angle);
                lineAnglesPath->moveTo(x, y);
                lineAnglesPath->lineTo(-x, -y);
            }
        }

        {
            SkPath* kindaTightQuad = &fPaths.push_back();
            kindaTightQuad->moveTo(0, -10 * SK_Scalar1);
            kindaTightQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -10 * SK_Scalar1, 0);
        }

        {
            SkPath* tightQuad = &fPaths.push_back();
            tightQuad->moveTo(0, -5 * SK_Scalar1);
            tightQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -5 * SK_Scalar1, 0);
        }

        {
            SkPath* tighterQuad = &fPaths.push_back();
            tighterQuad->moveTo(0, -2 * SK_Scalar1);
            tighterQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -2 * SK_Scalar1, 0);
        }

        {
            SkPath* unevenTighterQuad = &fPaths.push_back();
            unevenTighterQuad->moveTo(0, -1 * SK_Scalar1);
            SkPoint p;
            p.set(-2 * SK_Scalar1 + 3 * SkIntToScalar(102) / 4, SkIntToScalar(75));
            unevenTighterQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), p.fX, p.fY);
        }

        {
            SkPath* reallyTightQuad = &fPaths.push_back();
            reallyTightQuad->moveTo(0, -1 * SK_Scalar1);
            reallyTightQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), -1 * SK_Scalar1, 0);
        }

        {
            SkPath* closedQuad = &fPaths.push_back();
            closedQuad->moveTo(0, -0);
            closedQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100), 0, 0);
        }

        {
            SkPath* unevenClosedQuad = &fPaths.push_back();
            unevenClosedQuad->moveTo(0, -0);
            unevenClosedQuad->quadTo(SkIntToScalar(100), SkIntToScalar(100),
                                     SkIntToScalar(75), SkIntToScalar(75));
        }

        // Two problem cases for gpu hairline renderer found by shapeops testing. These used
        // to assert that the computed bounding box didn't contain all the vertices.
        {
            SkPath* problem1 = &fPaths.push_back();
            problem1->moveTo(SkIntToScalar(4), SkIntToScalar(6));
            problem1->cubicTo(SkIntToScalar(5), SkIntToScalar(6),
                              SkIntToScalar(5), SkIntToScalar(4),
                              SkIntToScalar(4), SkIntToScalar(0));
            problem1->close();
        }

        {
            SkPath* problem2 = &fPaths.push_back();
            problem2->moveTo(SkIntToScalar(5), SkIntToScalar(1));
            problem2->lineTo(4.32787323f, 1.67212653f);
            problem2->cubicTo(2.75223875f, 3.24776125f,
                              3.00581908f, 4.51236057f,
                              3.7580452f, 4.37367964f);
            problem2->cubicTo(4.66472578f, 3.888381f,
                              5.f, 2.875f,
                              5.f, 1.f);
            problem2->close();
        }

        // Three paths that show the same bug (missing end caps)
        {
            // A caret (crbug.com/131770)
            SkPath* bug0 = &fPaths.push_back();
            bug0->moveTo(6.5f,5.5f);
            bug0->lineTo(3.5f,0.5f);
            bug0->moveTo(0.5f,5.5f);
            bug0->lineTo(3.5f,0.5f);
        }

        {
            // An X (crbug.com/137317)
            SkPath* bug1 = &fPaths.push_back();

            bug1->moveTo(1, 1);
            bug1->lineTo(6, 6);
            bug1->moveTo(1, 6);
            bug1->lineTo(6, 1);
        }

        {
            // A right angle (crbug.com/137465 and crbug.com/256776)
            SkPath* bug2 = &fPaths.push_back();

            bug2->moveTo(5.5f, 5.5f);
            bug2->lineTo(5.5f, 0.5f);
            bug2->lineTo(0.5f, 0.5f);
        }

        {
            // Arc example to test imperfect truncation bug (crbug.com/295626)
            constexpr SkScalar kRad = SkIntToScalar(2000);
            constexpr SkScalar kStartAngle = 262.59717f;
            constexpr SkScalar kSweepAngle = SkScalarHalf(17.188717f);

            SkPath* bug = &fPaths.push_back();

            // Add a circular arc
            SkRect circle = SkRect::MakeLTRB(-kRad, -kRad, kRad, kRad);
            bug->addArc(circle, kStartAngle, kSweepAngle);

            // Now add the chord that should cap the circular arc
            SkScalar cosV, sinV = SkScalarSinCos(SkDegreesToRadians(kStartAngle), &cosV);

            SkPoint p0 = SkPoint::Make(kRad * cosV, kRad * sinV);

            sinV = SkScalarSinCos(SkDegreesToRadians(kStartAngle + kSweepAngle), &cosV);

            SkPoint p1 = SkPoint::Make(kRad * cosV, kRad * sinV);

            bug->moveTo(p0);
            bug->lineTo(p1);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        constexpr SkAlpha kAlphaValue[] = { 0xFF, 0x40 };
        constexpr SkScalar kWidths[] = { 0, 0.5f, 1.5f };

        enum {
            kMargin = 5,
        };
        int wrapX = 1250 - kMargin;

        SkScalar maxH = 0;
        canvas->translate(SkIntToScalar(kMargin), SkIntToScalar(kMargin));
        canvas->save();

        SkScalar x = SkIntToScalar(kMargin);
        for (int p = 0; p < fPaths.count(); ++p) {
            for (size_t a = 0; a < SK_ARRAY_COUNT(kAlphaValue); ++a) {
                for (int aa = 0; aa < 2; ++aa) {
                    for (size_t w = 0; w < SK_ARRAY_COUNT(kWidths); w++) {
                        const SkRect& bounds = fPaths[p].getBounds();

                        if (x + bounds.width() > wrapX) {
                            canvas->restore();
                            canvas->translate(0, maxH + SkIntToScalar(kMargin));
                            canvas->save();
                            maxH = 0;
                            x = SkIntToScalar(kMargin);
                        }

                        SkPaint paint;
                        paint.setARGB(kAlphaValue[a], 0, 0, 0);
                        paint.setAntiAlias(SkToBool(aa));
                        paint.setStyle(SkPaint::kStroke_Style);
                        paint.setStrokeWidth(kWidths[w]);

                        canvas->save();
                        canvas->translate(-bounds.fLeft, -bounds.fTop);
                        canvas->drawPath(fPaths[p], paint);
                        canvas->restore();

                        maxH = SkMaxScalar(maxH, bounds.height());

                        SkScalar dx = bounds.width() + SkIntToScalar(kMargin);
                        x += dx;
                        canvas->translate(dx, 0);
                    }
                }
            }
        }
        canvas->restore();
    }

private:
    SkTArray<SkPath> fPaths;
    typedef GM INHERITED;
};

static void draw_squarehair_tests(SkCanvas* canvas, SkScalar width, SkPaint::Cap cap, bool aa) {
    SkPaint paint;
    paint.setStrokeCap(cap);
    paint.setStrokeWidth(width);
    paint.setAntiAlias(aa);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->drawLine(10, 10, 20, 10, paint);
    canvas->drawLine(30, 10, 30, 20, paint);
    canvas->drawLine(40, 10, 50, 20, paint);
    SkPath path;
    path.moveTo(60, 10);
    path.quadTo(60, 20, 70, 20);
    path.conicTo(70, 10, 80, 10, 0.707f);
    canvas->drawPath(path, paint);
    path.reset();
    path.moveTo(90, 10);
    path.cubicTo(90, 20, 100, 20, 100, 10);
    path.lineTo(110, 10);
    canvas->drawPath(path, paint);
    canvas->translate(0, 30);
}

DEF_SIMPLE_GM(squarehair, canvas, 240, 360) {
    const bool aliases[] = { false, true };
    const SkScalar widths[] = { 0, 0.999f, 1, 1.001f };
    const SkPaint::Cap caps[] = { SkPaint::kButt_Cap, SkPaint::kSquare_Cap, SkPaint::kRound_Cap };
    for (auto alias : aliases) {
        canvas->save();
        for (auto width : widths) {
            for (auto cap : caps) {
                draw_squarehair_tests(canvas, width, cap, alias);
            }
        }
        canvas->restore();
        canvas->translate(120, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new HairlinesGM; }
static GMRegistry reg(MyFactory);

}
