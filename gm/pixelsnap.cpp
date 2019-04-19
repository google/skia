/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ToolUtils.h"
#include "gm.h"

#include "SkShader.h"

// This class of GMs test how edges/verts snap near rounding boundaries in device space without
// anti-aliaing.
class PixelSnapGM : public skiagm::GM {
public:
    PixelSnapGM() {}

protected:
    // kTrans should be even or checkboards wont agree in different test cases.
    static constexpr int kTrans = 14;
    static constexpr int kLabelPad = 4;
    // The inverse of this value should be a perfect SkScalar.
    static constexpr int kSubPixelSteps = 8;
    static constexpr int kLabelTextSize = 9;

    static_assert(kSubPixelSteps < 99, "label_offset_too_small");
    static constexpr int kLabelOffsetX = 2 * kLabelTextSize + kLabelPad;
    static constexpr int kLabelOffsetY = kLabelTextSize + kLabelPad;

    SkISize onISize() override {
        return SkISize::Make((kSubPixelSteps + 1) * kTrans + kLabelOffsetX + kLabelPad,
                             (kSubPixelSteps + 1) * kTrans + kLabelOffsetY + kLabelPad);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint bgPaint;
        bgPaint.setShader(ToolUtils::create_checkerboard_shader(0xFFAAAAAA, 0xFF777777, 1));
        canvas->drawPaint(bgPaint);

        SkString offset;
        SkPaint labelPaint;
        labelPaint.setColor(SK_ColorWHITE);
        SkFont  font(ToolUtils::create_portable_typeface(), SkIntToScalar(kLabelTextSize));
        SkPaint linePaint;
        linePaint.setColor(SK_ColorWHITE);

        // Draw row labels
        canvas->save();
            canvas->translate(0, SkIntToScalar(kLabelOffsetY));
            for (int i = 0; i <= kSubPixelSteps; ++i) {
                offset.printf("%d", i);
                canvas->drawString(offset, 0, i * kTrans + SkIntToScalar(kLabelTextSize),
                                   font, labelPaint);
            }
        canvas->restore();

        // Draw col labels
        canvas->save();
            canvas->translate(SkIntToScalar(kLabelOffsetX), 0);
            for (int i = 0; i <= kSubPixelSteps; ++i) {
                offset.printf("%d", i);
                canvas->drawString(offset, i * SkIntToScalar(kTrans), SkIntToScalar(kLabelTextSize),
                                   font, labelPaint);
            }
        canvas->restore();

        canvas->translate(SkIntToScalar(kLabelOffsetX), SkIntToScalar(kLabelOffsetY));

        // Draw test case grid lines (Draw them all at pixel centers to hopefully avoid any
        // snapping issues).
        for (int i = 0; i <= kSubPixelSteps + 1; ++i) {
            canvas->drawLine(0.5f,
                             i * SkIntToScalar(kTrans) + 0.5f,
                             SkIntToScalar(kTrans) * (kSubPixelSteps + 1) + 0.5f,
                             i * SkIntToScalar(kTrans) + 0.5f,
                             linePaint);
            canvas->drawLine(i * SkIntToScalar(kTrans) + 0.5f,
                             0.5f,
                             i * SkIntToScalar(kTrans) + 0.5f,
                             SkIntToScalar(kTrans) * (kSubPixelSteps + 1) + 0.5f,
                             linePaint);
        }

        for (int i = 0; i <= kSubPixelSteps; ++i) {
            for (int j = 0; j <= kSubPixelSteps; ++j) {
                canvas->save();
                // +1's account for the grid lines around each test case.
                canvas->translate(j * (kTrans + 1.f/kSubPixelSteps) + 1,
                                  i * (kTrans + 1.f/kSubPixelSteps) + 1);
                this->drawElement(canvas);
                canvas->restore();
            }
        }
    }

    virtual void drawElement(SkCanvas*) = 0;

private:
    typedef skiagm::GM INHERITED;
};

class PointSnapGM : public PixelSnapGM {
protected:
    SkString onShortName() override { return SkString("pixel_snap_point"); }
    void drawElement(SkCanvas* canvas) override {
        const SkPoint pt = { 1, 1 };
        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 1, &pt, paint);
    }

private:
    typedef PixelSnapGM INHERITED;
};

class LineSnapGM : public PixelSnapGM {
protected:
    SkString onShortName() override { return SkString("pixel_snap_line"); }
    void drawElement(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorGREEN);
        // Draw a horizontal and vertical line, each length 3.
        canvas->drawLine(1, 1, 4, 1, paint);
        canvas->drawLine(6, 1, 6, 4, paint);
    }

private:
    typedef PixelSnapGM INHERITED;
};

class RectSnapGM : public PixelSnapGM {
protected:
    SkString onShortName() override { return SkString("pixel_snap_rect"); }
    void drawElement(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawRect(SkRect::MakeXYWH(1, 1, 3, 3), paint);
    }

private:
    typedef PixelSnapGM INHERITED;
};

class ComboSnapGM : public PixelSnapGM {
protected:
    SkString onShortName() override { return SkString("pixel_snap_combo"); }
    void drawElement(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(false);
        // A rectangle that exactly covers a pixel, a point at each corner, 8 horiz/vert lines
        // at rect corners (two at each corner, extending away from rect). They are drawn in this
        // order lines (green), points (blue), rect(red).
        SkRect rect = SkRect::MakeXYWH(3, 3, 1, 1);
        paint.setColor(SK_ColorGREEN);
        const SkPoint lines[] = {
            { 3, 3 }, { 0, 3 },
            { 3, 3 }, { 3, 0 },
            { 4, 3 }, { 7, 3 },
            { 4, 3 }, { 4, 0 },
            { 3, 4 }, { 0, 4 },
            { 3, 4 }, { 3, 7 },
            { 4, 4 }, { 7, 4 },
            { 4, 4 }, { 4, 7 },
        };
        canvas->drawPoints(SkCanvas::kLines_PointMode, SK_ARRAY_COUNT(lines), lines, paint);

        const SkPoint pts[] = {
            { 4, 3 }, { 4, 4, }, { 3, 3 }, { 3, 4 },
        };
        paint.setColor(SK_ColorBLUE);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, SK_ARRAY_COUNT(pts), pts, paint);

        paint.setColor(SK_ColorRED);
        canvas->drawRect(rect, paint);
    }

private:
    typedef PixelSnapGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////
DEF_GM(return new PointSnapGM;)
DEF_GM(return new LineSnapGM;)
DEF_GM(return new RectSnapGM;)
DEF_GM(return new ComboSnapGM;)
