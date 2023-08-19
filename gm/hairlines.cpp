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

using namespace skia_private;

namespace skiagm {

class HairlinesGM : public GM {
protected:
    SkString getName() const override { return SkString("hairlines"); }

    SkISize getISize() override { return SkISize::Make(1250, 1250); }

    void onOnceBeforeDraw() override {
        {
            SkPathBuilder lineAngles;
            enum {
                kNumAngles = 15,
                kRadius = 40,
            };
            for (int i = 0; i < kNumAngles; ++i) {
                SkScalar angle = SK_ScalarPI * SkIntToScalar(i) / kNumAngles;
                SkScalar x = kRadius * SkScalarCos(angle);
                SkScalar y = kRadius * SkScalarSin(angle);
                lineAngles.moveTo(x, y).lineTo(-x, -y);
            }
            fPaths.push_back(lineAngles.detach());
        }

        fPaths.push_back(SkPathBuilder().moveTo(0, -10)
                                        .quadTo(100, 100, -10, 0)
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(0, -5)
                                        .quadTo(100, 100, -5, 0)
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(0, -2)
                                        .quadTo(100, 100, -2, 0)
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(0, -1)
                                        .quadTo(100, 100, -2 + 306.0f / 4, 75)
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(0, -1)
                                        .quadTo(100, 100, -1, 0)
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(0, -0)
                                        .quadTo(100, 100, 0, 0)
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(0, -0)
                                        .quadTo(100, 100, 75, 75)
                                        .detach());

        // Two problem cases for gpu hairline renderer found by shapeops testing. These used
        // to assert that the computed bounding box didn't contain all the vertices.

        fPaths.push_back(SkPathBuilder().moveTo(4, 6)
                                        .cubicTo(5, 6, 5, 4, 4, 0)
                                        .close()
                                        .detach());

        fPaths.push_back(SkPathBuilder().moveTo(5, 1)
                                        .lineTo( 4.32787323f, 1.67212653f)
                                        .cubicTo(2.75223875f, 3.24776125f,
                                                 3.00581908f, 4.51236057f,
                                                 3.7580452f,  4.37367964f)
                                        .cubicTo(4.66472578f, 3.888381f,
                                                 5.f,         2.875f,
                                                 5.f,         1.f)
                                        .close()
                                        .detach());

        // Three paths that show the same bug (missing end caps)

        fPaths.push_back(SkPathBuilder().moveTo(6.5f,5.5f)
                                        .lineTo(3.5f,0.5f)
                                        .moveTo(0.5f,5.5f)
                                        .lineTo(3.5f,0.5f)
                                        .detach());

        // An X (crbug.com/137317)
        fPaths.push_back(SkPathBuilder().moveTo(1, 1)
                                        .lineTo(6, 6)
                                        .moveTo(1, 6)
                                        .lineTo(6, 1)
                                        .detach());

        // A right angle (crbug.com/137465 and crbug.com/256776)
        fPaths.push_back(SkPathBuilder().moveTo(5.5f, 5.5f)
                                        .lineTo(5.5f, 0.5f)
                                        .lineTo(0.5f, 0.5f)
                                        .detach());

        {
            // Arc example to test imperfect truncation bug (crbug.com/295626)
            constexpr SkScalar kRad = SkIntToScalar(2000);
            constexpr SkScalar kStartAngle = 262.59717f;
            constexpr SkScalar kSweepAngle = SkScalarHalf(17.188717f);

            SkPathBuilder bug;

            // Add a circular arc
            SkRect circle = SkRect::MakeLTRB(-kRad, -kRad, kRad, kRad);
            bug.addArc(circle, kStartAngle, kSweepAngle);

            // Now add the chord that should cap the circular arc
            SkPoint p0 = { kRad * SkScalarCos(SkDegreesToRadians(kStartAngle)),
                           kRad * SkScalarSin(SkDegreesToRadians(kStartAngle)) };

            SkPoint p1 = { kRad * SkScalarCos(SkDegreesToRadians(kStartAngle + kSweepAngle)),
                           kRad * SkScalarSin(SkDegreesToRadians(kStartAngle + kSweepAngle)) };

            bug.moveTo(p0);
            bug.lineTo(p1);
            fPaths.push_back(bug.detach());
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
        for (int p = 0; p < fPaths.size(); ++p) {
            for (size_t a = 0; a < std::size(kAlphaValue); ++a) {
                for (int aa = 0; aa < 2; ++aa) {
                    for (size_t w = 0; w < std::size(kWidths); w++) {
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

                        maxH = std::max(maxH, bounds.height());

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
    TArray<SkPath> fPaths;
    using INHERITED = GM;
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
    SkPathBuilder path;
    path.moveTo(60, 10);
    path.quadTo(60, 20, 70, 20);
    path.conicTo(70, 10, 80, 10, 0.707f);
    canvas->drawPath(path.detach(), paint);

    path.moveTo(90, 10);
    path.cubicTo(90, 20, 100, 20, 100, 10);
    path.lineTo(110, 10);
    canvas->drawPath(path.detach(), paint);
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

// GM to test subdivision of hairlines
static void draw_subdivided_quad(SkCanvas* canvas, int x0, int y0, int x1, int y1, SkColor color) {
    SkPaint paint;
    paint.setStrokeWidth(1);
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(color);

    canvas->drawPath(SkPathBuilder().moveTo(0,0)
                                    .quadTo(SkIntToScalar(x0), SkIntToScalar(y0),
                                            SkIntToScalar(x1), SkIntToScalar(y1))
                                    .detach(),
                     paint);
}

DEF_SIMPLE_GM(hairline_subdiv, canvas, 512, 256) {
    // no subdivisions
    canvas->translate(45, -25);
    draw_subdivided_quad(canvas, 334, 334, 467, 267, SK_ColorBLACK);

    // one subdivision
    canvas->translate(-185, -150);
    draw_subdivided_quad(canvas, 472, 472, 660, 378, SK_ColorRED);

    // two subdivisions
    canvas->translate(-275, -200);
    draw_subdivided_quad(canvas, 668, 668, 934, 535, SK_ColorGREEN);

    // three subdivisions
    canvas->translate(-385, -260);
    draw_subdivided_quad(canvas, 944, 944, 1320, 756, SK_ColorBLUE);
}

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new HairlinesGM; )

}  // namespace skiagm
