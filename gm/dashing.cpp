/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkDashPathEffect.h"

static void drawline(SkCanvas* canvas, int on, int off, const SkPaint& paint,
                     SkScalar finalX = SkIntToScalar(600), SkScalar finalY = SkIntToScalar(0),
                     SkScalar phase = SkIntToScalar(0),
                     SkScalar startX = SkIntToScalar(0), SkScalar startY = SkIntToScalar(0)) {
    SkPaint p(paint);

    const SkScalar intervals[] = {
        SkIntToScalar(on),
        SkIntToScalar(off),
    };

    p.setPathEffect(SkDashPathEffect::Make(intervals, 2, phase));
    canvas->drawLine(startX, startY, finalX, finalY, p);
}

// earlier bug stopped us from drawing very long single-segment dashes, because
// SkPathMeasure was skipping very small delta-T values (nearlyzero). This is
// now fixes, so this giant dash should appear.
static void show_giant_dash(SkCanvas* canvas) {
    SkPaint paint;

    drawline(canvas, 1, 1, paint, SkIntToScalar(20 * 1000));
}

static void show_zero_len_dash(SkCanvas* canvas) {
    SkPaint paint;

    drawline(canvas, 2, 2, paint, SkIntToScalar(0));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(2));
    canvas->translate(0, SkIntToScalar(20));
    drawline(canvas, 4, 4, paint, SkIntToScalar(0));
}

class DashingGM : public skiagm::GM {
public:
    DashingGM() {}

protected:

    SkString onShortName() {
        return SkString("dashing");
    }

    SkISize onISize() { return SkISize::Make(640, 340); }

    virtual void onDraw(SkCanvas* canvas) {
        static const struct {
            int fOnInterval;
            int fOffInterval;
        } gData[] = {
            { 1, 1 },
            { 4, 1 },
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        canvas->translate(0, SK_ScalarHalf);
        for (int width = 0; width <= 2; ++width) {
            for (size_t data = 0; data < SK_ARRAY_COUNT(gData); ++data) {
                for (int aa = 0; aa <= 1; ++aa) {
                    int w = width * width * width;
                    paint.setAntiAlias(SkToBool(aa));
                    paint.setStrokeWidth(SkIntToScalar(w));

                    int scale = w ? w : 1;

                    drawline(canvas, gData[data].fOnInterval * scale,
                             gData[data].fOffInterval * scale,
                             paint);
                    canvas->translate(0, SkIntToScalar(20));
                }
            }
        }

        show_giant_dash(canvas);
        canvas->translate(0, SkIntToScalar(20));
        show_zero_len_dash(canvas);
        canvas->translate(0, SkIntToScalar(20));
        // Draw 0 on, 0 off dashed line
        paint.setStrokeWidth(SkIntToScalar(8));
        drawline(canvas, 0, 0, paint);
    }
};

///////////////////////////////////////////////////////////////////////////////

static void make_unit_star(SkPath* path, int n) {
    SkScalar rad = -SK_ScalarPI / 2;
    const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

    path->moveTo(0, -SK_Scalar1);
    for (int i = 1; i < n; i++) {
        rad += drad;
        SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
        path->lineTo(cosV, sinV);
    }
    path->close();
}

static void make_path_line(SkPath* path, const SkRect& bounds) {
    path->moveTo(bounds.left(), bounds.top());
    path->lineTo(bounds.right(), bounds.bottom());
}

static void make_path_rect(SkPath* path, const SkRect& bounds) {
    path->addRect(bounds);
}

static void make_path_oval(SkPath* path, const SkRect& bounds) {
    path->addOval(bounds);
}

static void make_path_star(SkPath* path, const SkRect& bounds) {
    make_unit_star(path, 5);
    SkMatrix matrix;
    matrix.setRectToRect(path->getBounds(), bounds, SkMatrix::kCenter_ScaleToFit);
    path->transform(matrix);
}

class Dashing2GM : public skiagm::GM {
public:
    Dashing2GM() {}

protected:

    SkString onShortName() {
        return SkString("dashing2");
    }

    SkISize onISize() { return SkISize::Make(640, 480); }

    virtual void onDraw(SkCanvas* canvas) {
        static const int gIntervals[] = {
            3,  // 3 dashes: each count [0] followed by intervals [1..count]
            2,  10, 10,
            4,  20, 5, 5, 5,
            2,  2, 2
        };

        void (*gProc[])(SkPath*, const SkRect&) = {
            make_path_line, make_path_rect, make_path_oval, make_path_star,
        };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(6));

        SkRect bounds = SkRect::MakeWH(SkIntToScalar(120), SkIntToScalar(120));
        bounds.offset(SkIntToScalar(20), SkIntToScalar(20));
        SkScalar dx = bounds.width() * 4 / 3;
        SkScalar dy = bounds.height() * 4 / 3;

        const int* intervals = &gIntervals[1];
        for (int y = 0; y < gIntervals[0]; ++y) {
            SkScalar vals[SK_ARRAY_COUNT(gIntervals)];  // more than enough
            int count = *intervals++;
            for (int i = 0; i < count; ++i) {
                vals[i] = SkIntToScalar(*intervals++);
            }
            SkScalar phase = vals[0] / 2;
            paint.setPathEffect(SkDashPathEffect::Make(vals, count, phase));

            for (size_t x = 0; x < SK_ARRAY_COUNT(gProc); ++x) {
                SkPath path;
                SkRect r = bounds;
                r.offset(x * dx, y * dy);
                gProc[x](&path, r);

                canvas->drawPath(path, paint);
            }
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

// Test out the on/off line dashing Chrome if fond of
class Dashing3GM : public skiagm::GM {
public:
    Dashing3GM() {}

protected:

    SkString onShortName() {
        return SkString("dashing3");
    }

    SkISize onISize() { return SkISize::Make(640, 480); }

    // Draw a 100x100 block of dashed lines. The horizontal ones are BW
    // while the vertical ones are AA.
    void drawDashedLines(SkCanvas* canvas,
                         SkScalar lineLength,
                         SkScalar phase,
                         SkScalar dashLength,
                         int strokeWidth,
                         bool circles) {
        SkPaint p;
        p.setColor(SK_ColorBLACK);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SkIntToScalar(strokeWidth));

        if (circles) {
            p.setStrokeCap(SkPaint::kRound_Cap);
        }

        SkScalar intervals[2] = { dashLength, dashLength };

        p.setPathEffect(SkDashPathEffect::Make(intervals, 2, phase));

        SkPoint pts[2];

        for (int y = 0; y < 100; y += 10*strokeWidth) {
            pts[0].set(0, SkIntToScalar(y));
            pts[1].set(lineLength, SkIntToScalar(y));

            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, p);
        }

        p.setAntiAlias(true);

        for (int x = 0; x < 100; x += 14*strokeWidth) {
            pts[0].set(SkIntToScalar(x), 0);
            pts[1].set(SkIntToScalar(x), lineLength);

            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, p);
        }
    }

    virtual void onDraw(SkCanvas* canvas) {
        // 1on/1off 1x1 squares with phase of 0 - points fastpath
        canvas->save();
            canvas->translate(2, 0);
            this->drawDashedLines(canvas, 100, 0, SK_Scalar1, 1, false);
        canvas->restore();

        // 1on/1off 1x1 squares with phase of .5 - rects fastpath (due to partial squares)
        canvas->save();
            canvas->translate(112, 0);
            this->drawDashedLines(canvas, 100, SK_ScalarHalf, SK_Scalar1, 1, false);
        canvas->restore();

        // 1on/1off 1x1 squares with phase of 1 - points fastpath
        canvas->save();
            canvas->translate(222, 0);
            this->drawDashedLines(canvas, 100, SK_Scalar1, SK_Scalar1, 1, false);
        canvas->restore();

        // 1on/1off 1x1 squares with phase of 1 and non-integer length - rects fastpath
        canvas->save();
            canvas->translate(332, 0);
            this->drawDashedLines(canvas, 99.5f, SK_ScalarHalf, SK_Scalar1, 1, false);
        canvas->restore();

        // 255on/255off 1x1 squares with phase of 0 - rects fast path
        canvas->save();
            canvas->translate(446, 0);
            this->drawDashedLines(canvas, 100, 0, SkIntToScalar(255), 1, false);
        canvas->restore();

        // 1on/1off 3x3 squares with phase of 0 - points fast path
        canvas->save();
            canvas->translate(2, 110);
            this->drawDashedLines(canvas, 100, 0, SkIntToScalar(3), 3, false);
        canvas->restore();

        // 1on/1off 3x3 squares with phase of 1.5 - rects fast path
        canvas->save();
            canvas->translate(112, 110);
            this->drawDashedLines(canvas, 100, 1.5f, SkIntToScalar(3), 3, false);
        canvas->restore();

        // 1on/1off 1x1 circles with phase of 1 - no fast path yet
        canvas->save();
            canvas->translate(2, 220);
            this->drawDashedLines(canvas, 100, SK_Scalar1, SK_Scalar1, 1, true);
        canvas->restore();

        // 1on/1off 3x3 circles with phase of 1 - no fast path yet
        canvas->save();
            canvas->translate(112, 220);
            this->drawDashedLines(canvas, 100, 0, SkIntToScalar(3), 3, true);
        canvas->restore();

        // 1on/1off 1x1 squares with rotation - should break fast path
        canvas->save();
            canvas->translate(332+SK_ScalarRoot2Over2*100, 110+SK_ScalarRoot2Over2*100);
            canvas->rotate(45);
            canvas->translate(-50, -50);

            this->drawDashedLines(canvas, 100, SK_Scalar1, SK_Scalar1, 1, false);
        canvas->restore();

        // 3on/3off 3x1 rects - should use rect fast path regardless of phase
        for (int phase = 0; phase <= 3; ++phase) {
            canvas->save();
                canvas->translate(SkIntToScalar(phase*110+2),
                                  SkIntToScalar(330));
                this->drawDashedLines(canvas, 100, SkIntToScalar(phase), SkIntToScalar(3), 1, false);
            canvas->restore();
        }
    }

};

//////////////////////////////////////////////////////////////////////////////

class Dashing4GM : public skiagm::GM {
public:
    Dashing4GM() {}

protected:

    SkString onShortName() {
        return SkString("dashing4");
    }

    SkISize onISize() { return SkISize::Make(640, 950); }

    virtual void onDraw(SkCanvas* canvas) {
        static const struct {
            int fOnInterval;
            int fOffInterval;
        } gData[] = {
            { 1, 1 },
            { 4, 2 },
            { 0, 4 }, // test for zero length on interval
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        canvas->translate(0, SK_ScalarHalf);

        for (int width = 0; width <= 2; ++width) {
            for (size_t data = 0; data < SK_ARRAY_COUNT(gData); ++data) {
                for (int aa = 0; aa <= 1; ++aa) {
                    for (int cap = 0; cap <= 1; ++cap) {
                        int w = width * width * width;
                        paint.setAntiAlias(SkToBool(aa));
                        paint.setStrokeWidth(SkIntToScalar(w));

                        SkToBool(cap) ? paint.setStrokeCap(SkPaint::kSquare_Cap)
                            : paint.setStrokeCap(SkPaint::kRound_Cap);

                        int scale = w ? w : 1;

                        drawline(canvas, gData[data].fOnInterval * scale,
                                 gData[data].fOffInterval * scale,
                                 paint);
                        canvas->translate(0, SkIntToScalar(20));
                    }
                }
            }
        }

        for (int aa = 0; aa <= 1; ++aa) {
            paint.setAntiAlias(SkToBool(aa));
            paint.setStrokeWidth(8.f);
            paint.setStrokeCap(SkPaint::kSquare_Cap);
            // Single dash element that is cut off at start and end
            drawline(canvas, 32, 16, paint, 20.f, 0, 5.f);
            canvas->translate(0, SkIntToScalar(20));

            // Two dash elements where each one is cut off at beginning and end respectively
            drawline(canvas, 32, 16, paint, 56.f, 0, 5.f);
            canvas->translate(0, SkIntToScalar(20));

            // Many dash elements where first and last are cut off at beginning and end respectively
            drawline(canvas, 32, 16, paint, 584.f, 0, 5.f);
            canvas->translate(0, SkIntToScalar(20));

            // Diagonal dash line where src pnts are not axis aligned (as apposed to being diagonal from
            // a canvas rotation)
            drawline(canvas, 32, 16, paint, 600.f, 30.f);
            canvas->translate(0, SkIntToScalar(20));

            // Case where only the off interval exists on the line. Thus nothing should be drawn
            drawline(canvas, 32, 16, paint, 8.f, 0.f, 40.f);
            canvas->translate(0, SkIntToScalar(20));
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

class Dashing5GM : public skiagm::GM {
public:
    Dashing5GM(bool doAA) : fDoAA(doAA) {}

protected:

    bool runAsBench() const override { return true; }

    SkString onShortName() override {
        if (fDoAA) {
            return SkString("dashing5_aa");
        } else {
            return SkString("dashing5_bw");
        }
    }

    SkISize onISize() override { return SkISize::Make(400, 200); }

    void onDraw(SkCanvas* canvas) override {
        static const int kOn = 4;
        static const int kOff = 4;
        static const int kIntervalLength = kOn + kOff;

        static const SkColor gColors[kIntervalLength] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorCYAN,
            SK_ColorMAGENTA,
            SK_ColorYELLOW,
            SK_ColorGRAY,
            SK_ColorDKGRAY
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);

        paint.setAntiAlias(fDoAA);

        SkMatrix rot;
        rot.setRotate(90);
        SkASSERT(rot.rectStaysRect());

        canvas->concat(rot);

        int sign;       // used to toggle the direction of the lines
        int phase = 0;

        for (int x = 0; x < 200; x += 10) {
            paint.setStrokeWidth(SkIntToScalar(phase+1));
            paint.setColor(gColors[phase]);
            sign = (x % 20) ? 1 : -1;
            drawline(canvas, kOn, kOff, paint,
                     SkIntToScalar(x), -sign * SkIntToScalar(10003),
                     SkIntToScalar(phase),
                     SkIntToScalar(x),  sign * SkIntToScalar(10003));
            phase = (phase + 1) % kIntervalLength;
        }

        for (int y = -400; y < 0; y += 10) {
            paint.setStrokeWidth(SkIntToScalar(phase+1));
            paint.setColor(gColors[phase]);
            sign = (y % 20) ? 1 : -1;
            drawline(canvas, kOn, kOff, paint,
                     -sign * SkIntToScalar(10003), SkIntToScalar(y),
                     SkIntToScalar(phase),
                      sign * SkIntToScalar(10003), SkIntToScalar(y));
            phase = (phase + 1) % kIntervalLength;
        }
    }

private:
    bool fDoAA;
};

DEF_SIMPLE_GM(longpathdash, canvas, 612, 612) {
    SkPath lines;
    for (int x = 32; x < 256; x += 16) {
        for (SkScalar a = 0; a < 3.141592f * 2; a += 0.03141592f) {
            SkPoint pts[2] = {
                { 256 + (float) sin(a) * x,
                  256 + (float) cos(a) * x },
                { 256 + (float) sin(a + 3.141592 / 3) * (x + 64),
                  256 + (float) cos(a + 3.141592 / 3) * (x + 64) }
            };
            lines.moveTo(pts[0]);
            for (SkScalar i = 0; i < 1; i += 0.05f) {
                lines.lineTo(pts[0].fX * (1 - i) + pts[1].fX * i,
                             pts[0].fY * (1 - i) + pts[1].fY * i);
            }
        }
    }
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(1);
    const SkScalar intervals[] = { 1, 1 };
    p.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
    
    canvas->translate(50, 50);
    canvas->drawPath(lines, p);
}

DEF_SIMPLE_GM(longlinedash, canvas, 512, 512) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(80);

    const SkScalar intervals[] = { 2, 2 };
    p.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
    canvas->drawRect(SkRect::MakeXYWH(-10000, 100, 20000, 20), p);
}

DEF_SIMPLE_GM(longwavyline, canvas, 512, 512) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(2);

    SkPath wavy;
    wavy.moveTo(-10000, 100);
    for (SkScalar i = -10000; i < 10000; i += 20) {
        wavy.quadTo(i + 5, 95, i + 10, 100);
        wavy.quadTo(i + 15, 105, i + 20, 100);
    }
    canvas->drawPath(wavy, p);
}

DEF_SIMPLE_GM(dashtextcaps, canvas, 512, 512) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(10);
    p.setStrokeCap(SkPaint::kRound_Cap);
    p.setStrokeJoin(SkPaint::kRound_Join);
    p.setTextSize(100);
    p.setARGB(0xff, 0xbb, 0x00, 0x00);
    sk_tool_utils::set_portable_typeface(&p);
    const SkScalar intervals[] = { 12, 12 };
    p.setPathEffect(SkDashPathEffect::Make(intervals, SK_ARRAY_COUNT(intervals), 0));
    canvas->drawText("Sausages", 8, 10, 90, p);
    canvas->drawLine(8, 120, 456, 120, p);
}

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DashingGM;)
DEF_GM(return new Dashing2GM;)
DEF_GM(return new Dashing3GM;)
DEF_GM(return new Dashing4GM;)
DEF_GM(return new Dashing5GM(true);)
DEF_GM(return new Dashing5GM(false);)
