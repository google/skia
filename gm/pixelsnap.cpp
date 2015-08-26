/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkShader.h"

// This class of GMs test how edges/verts snap near rounding boundaries in device space without
// anti-aliaing.
class PixelSnapGM : public skiagm::GM {
public:
    PixelSnapGM() {}

protected:
    // kTrans should be even or checkboards wont agree in different test cases.
    static const int kTrans = 14;
    static const int kLabelPad = 4;
    // The inverse of this value should be a perfect SkScalar.
    static const int kSubPixelSteps = 8;
    static const int kLabelTextSize = 9;

    static_assert(kSubPixelSteps < 99, "label_offset_too_small");
    static const int kLabelOffsetX = 2 * kLabelTextSize + kLabelPad;
    static const int kLabelOffsetY = kLabelTextSize + kLabelPad;

    SkISize onISize() override {
        return SkISize::Make((kSubPixelSteps + 1) * kTrans + kLabelOffsetX + kLabelPad,
                             (kSubPixelSteps + 1) * kTrans + kLabelOffsetY + kLabelPad);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint bgPaint;
        bgPaint.setShader(
                sk_tool_utils::create_checkerboard_shader(
                sk_tool_utils::color_to_565(0xFFAAAAAA), 
                sk_tool_utils::color_to_565(0xFF777777), 1))->unref();
        canvas->drawPaint(bgPaint);

        SkString offset;
        SkPaint labelPaint;
        labelPaint.setAntiAlias(true);
        labelPaint.setColor(SK_ColorWHITE);
        labelPaint.setTextSize(SkIntToScalar(kLabelTextSize));
        sk_tool_utils::set_portable_typeface(&labelPaint);
        SkPaint linePaint;
        linePaint.setColor(SK_ColorWHITE);

        // Draw row labels
        canvas->save();
            canvas->translate(0, SkIntToScalar(kLabelOffsetY));
            for (int i = 0; i <= kSubPixelSteps; ++i) {
                offset.printf("%d", i);
                canvas->drawText(offset.c_str(), offset.size(),
                                    0, i * kTrans + labelPaint.getTextSize(),
                                    labelPaint);
            }
        canvas->restore();

        // Draw col labels
        canvas->save();
            canvas->translate(SkIntToScalar(kLabelOffsetX), 0);
            for (int i = 0; i <= kSubPixelSteps; ++i) {
                offset.printf("%d", i);
                canvas->drawText(offset.c_str(), offset.size(),
                                    i * SkIntToScalar(kTrans), labelPaint.getTextSize(),
                                    labelPaint);
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
    void drawElement(SkCanvas* canvas) override { canvas->drawPoint(1, 1, SK_ColorBLUE); }

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
        canvas->drawLine(3, 3, 0, 3, paint);
        canvas->drawLine(3, 3, 3, 0, paint);
        canvas->drawLine(4, 3, 7, 3, paint);
        canvas->drawLine(4, 3, 4, 0, paint);
        canvas->drawLine(3, 4, 0, 4, paint);
        canvas->drawLine(3, 4, 3, 7, paint);
        canvas->drawLine(4, 4, 7, 4, paint);
        canvas->drawLine(4, 4, 4, 7, paint);
        canvas->drawPoint(4, 3, SK_ColorBLUE);
        canvas->drawPoint(4, 4, SK_ColorBLUE);
        canvas->drawPoint(3, 3, SK_ColorBLUE);
        canvas->drawPoint(3, 4, SK_ColorBLUE);
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
