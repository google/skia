/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkPathMeasure.h"
#include "SkRandom.h"

class AddArcGM : public skiagm::GM {
public:
    AddArcGM() : fRotate(0) {}

protected:
    SkString onShortName() override { return SkString("addarc"); }

    SkISize onISize() override { return SkISize::Make(1040, 1040); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(20, 20);

        SkRect r = SkRect::MakeWH(1000, 1000);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(15);

        const SkScalar inset = paint.getStrokeWidth() + 4;
        const SkScalar sweepAngle = 345;
        SkRandom rand;

        SkScalar sign = 1;
        while (r.width() > paint.getStrokeWidth() * 3) {
            paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
            SkScalar startAngle = rand.nextUScalar1() * 360;

            SkScalar speed = SkScalarSqrt(16 / r.width()) * 0.5f;
            startAngle += fRotate * 360 * speed * sign;

            SkPath path;
            path.addArc(r, startAngle, sweepAngle);
            canvas->drawPath(path, paint);

            r.inset(inset, inset);
            sign = -sign;
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fRotate = timer.scaled(1, 360);
        return true;
    }

private:
    SkScalar fRotate;
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new AddArcGM; )

///////////////////////////////////////////////////

#define R   400

class AddArcMeasGM : public skiagm::GM {
public:
    AddArcMeasGM() {}

protected:
    SkString onShortName() override { return SkString("addarc_meas"); }

    SkISize onISize() override { return SkISize::Make(2*R + 40, 2*R + 40); }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(R + 20, R + 20);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        SkPaint measPaint;
        measPaint.setAntiAlias(true);
        measPaint.setColor(SK_ColorRED);

        const SkRect oval = SkRect::MakeLTRB(-R, -R, R, R);
        canvas->drawOval(oval, paint);

        for (SkScalar deg = 0; deg < 360; deg += 10) {
            const SkScalar rad = SkDegreesToRadians(deg);
            SkScalar rx = SkScalarCos(rad) * R;
            SkScalar ry = SkScalarSin(rad) * R;

            canvas->drawLine(0, 0, rx, ry, paint);

            SkPath path;
            path.addArc(oval, 0, deg);
            SkPathMeasure meas(path, false);
            SkScalar arcLen = rad * R;
            SkPoint pos;
            if (meas.getPosTan(arcLen, &pos, nullptr)) {
                canvas->drawLine({0, 0}, pos, measPaint);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new AddArcMeasGM; )

///////////////////////////////////////////////////

// Emphasize drawing a stroked oval (containing conics) and then scaling the results up,
// to ensure that we compute the stroke taking the CTM into account
//
class StrokeCircleGM : public skiagm::GM {
public:
    StrokeCircleGM() : fRotate(0) {}

protected:
    SkString onShortName() override { return SkString("strokecircle"); }

    SkISize onISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(20, 20);
        canvas->translate(13, 13);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1 / 2);

        const SkScalar delta = paint.getStrokeWidth() * 3 / 2;
        SkRect r = SkRect::MakeXYWH(-12, -12, 24, 24);
        SkRandom rand;

        SkScalar sign = 1;
        while (r.width() > paint.getStrokeWidth() * 2) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(fRotate * sign);

            paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fRotate = timer.scaled(60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new StrokeCircleGM; )

//////////////////////

// Fill circles and rotate them to test our Analytic Anti-Aliasing.
// This test is based on StrokeCircleGM.
class FillCircleGM : public skiagm::GM {
public:
    FillCircleGM() : fRotate(0) {}

protected:
    SkString onShortName() override { return SkString("fillcircle"); }

    SkISize onISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(20, 20);
        canvas->translate(13, 13);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SK_Scalar1 / 2);

        const SkScalar strokeWidth = paint.getStrokeWidth();
        const SkScalar delta = strokeWidth * 3 / 2;
        SkRect r = SkRect::MakeXYWH(-12, -12, 24, 24);
        SkRandom rand;

        // Reset style to fill. We only need stroke stype for producing delta and strokeWidth
        paint.setStyle(SkPaint::kFill_Style);

        SkScalar sign = 1;
        while (r.width() > strokeWidth * 2) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(fRotate * sign);
            paint.setColor(sk_tool_utils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fRotate = timer.scaled(60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new FillCircleGM; )

//////////////////////

static void html_canvas_arc(SkPath* path, SkScalar x, SkScalar y, SkScalar r, SkScalar start,
                            SkScalar end, bool ccw, bool callArcTo) {
    SkRect bounds = { x - r, y - r, x + r, y + r };
    SkScalar sweep = ccw ? end - start : start - end;
    if (callArcTo)
        path->arcTo(bounds, start, sweep, false);
    else
        path->addArc(bounds, start, sweep);
}

// Lifted from canvas-arc-circumference-fill-diffs.html
class ManyArcsGM : public skiagm::GM {
public:
    ManyArcsGM() {}

protected:
    SkString onShortName() override { return SkString("manyarcs"); }

    SkISize onISize() override { return SkISize::Make(620, 330); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(10, 10);

        // 20 angles.
        SkScalar sweepAngles[] = {
                           -123.7f, -2.3f, -2, -1, -0.3f, -0.000001f, 0, 0.000001f, 0.3f, 0.7f,
                           1, 1.3f, 1.5f, 1.7f, 1.99999f, 2, 2.00001f, 2.3f, 4.3f, 3934723942837.3f
        };
        for (size_t i = 0; i < SK_ARRAY_COUNT(sweepAngles); ++i) {
            sweepAngles[i] *= 180;
        }

        SkScalar startAngles[] = { -1, -0.5f, 0, 0.5f };
        for (size_t i = 0; i < SK_ARRAY_COUNT(startAngles); ++i) {
            startAngles[i] *= 180;
        }

        bool anticlockwise = false;
        SkScalar sign = 1;
        for (size_t i = 0; i < SK_ARRAY_COUNT(startAngles) * 2; ++i) {
            if (i == SK_ARRAY_COUNT(startAngles)) {
                anticlockwise = true;
                sign = -1;
            }
            SkScalar startAngle = startAngles[i % SK_ARRAY_COUNT(startAngles)] * sign;
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(sweepAngles); ++j) {
                SkPath path;
                path.moveTo(0, 2);
                html_canvas_arc(&path, 18, 15, 10, startAngle, startAngle + (sweepAngles[j] * sign),
                                anticlockwise, true);
                path.lineTo(0, 28);
                canvas->drawPath(path, paint);
                canvas->translate(30, 0);
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new ManyArcsGM; )

// Lifted from https://bugs.chromium.org/p/chromium/issues/detail?id=640031
class TinyAngleBigRadiusArcsGM : public skiagm::GM {
public:
    TinyAngleBigRadiusArcsGM() {}

protected:
    SkString onShortName() override { return SkString("tinyanglearcs"); }

    SkISize onISize() override { return SkISize::Make(620, 330); }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        canvas->translate(50, 50);

        SkScalar outerRadius = 100000.0f;
        SkScalar innerRadius = outerRadius - 20.0f;
        SkScalar centerX = 50;
        SkScalar centerY = outerRadius;
        SkScalar startAngles[] = { 1.5f * SK_ScalarPI , 1.501f * SK_ScalarPI  };
        SkScalar sweepAngle = 10.0f / outerRadius;

        for (size_t i = 0; i < SK_ARRAY_COUNT(startAngles); ++i) {
            SkPath path;
            SkScalar endAngle = startAngles[i] + sweepAngle;
            path.moveTo(centerX + innerRadius * sk_float_cos(startAngles[i]),
                        centerY + innerRadius * sk_float_sin(startAngles[i]));
            path.lineTo(centerX + outerRadius * sk_float_cos(startAngles[i]),
                        centerY + outerRadius * sk_float_sin(startAngles[i]));
            // A combination of tiny sweepAngle + large radius, we should draw a line.
            html_canvas_arc(&path, centerX, outerRadius, outerRadius,
                            startAngles[i] * 180 / SK_ScalarPI, endAngle * 180 / SK_ScalarPI,
                            true, true);
            path.lineTo(centerX + innerRadius * sk_float_cos(endAngle),
                        centerY + innerRadius * sk_float_sin(endAngle));
            html_canvas_arc(&path, centerX, outerRadius, innerRadius,
                            endAngle * 180 / SK_ScalarPI, startAngles[i] * 180 / SK_ScalarPI,
                            true, false);
            canvas->drawPath(path, paint);
            canvas->translate(20, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new TinyAngleBigRadiusArcsGM; )


DEF_SIMPLE_GM(dither1, canvas, 256, 256) {
    SkBitmap bm16;
    bm16.allocPixels(SkImageInfo::Make(32, 32, kRGB_565_SkColorType, kOpaque_SkAlphaType));
    SkCanvas c16(bm16);
    SkPaint colorPaint;
    for (auto dither : { false, true } ) {
        colorPaint.setDither(dither);
        for (auto colors : { 0xFF333333, 0xFF666666, 0xFF999999, 0xFFCCCCCC } ) {
            for (auto mask : { 0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFFFF } ) {
                 colorPaint.setColor(colors & mask);
                 c16.drawRect({0, 0, 8, 4}, colorPaint);
                 c16.translate(8, 0);
            }
            c16.translate(-32, 4);
        }
    }
    canvas->scale(8, 8);
    canvas->drawBitmap(bm16, 0, 0);
}

#include "SkGradientShader.h"

DEF_SIMPLE_GM(dither2, canvas, 256, 256) {
    SkBitmap bm32;
    bm32.allocPixels(SkImageInfo::Make(20, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkCanvas c32(bm32);
    SkPoint points[] = {{0, 0}, {20, 0}};
    SkColor colors[] = {0xFF334455, 0xFF662211 };
    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(
                     points, colors, nullptr, SK_ARRAY_COUNT(colors),
                     SkShader::kClamp_TileMode, 0, nullptr));
    paint.setDither(true);
    c32.drawPaint(paint);
    canvas->scale(12, 12);
    canvas->clear(0);
    canvas->drawBitmap(bm32, 0, 0);
    paint.setBlendMode(SkBlendMode::kPlus);
    canvas->drawBitmap(bm32, 0, 11, &paint);
    canvas->drawBitmap(bm32, 0, 11, &paint);
    canvas->drawBitmap(bm32, 0, 11, &paint);
}

#include "SkTextBlob.h"
#include <vector>

DEF_SIMPLE_GM(textblobinterval, canvas, 256, 256) {
            SkPaint paint;
            paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
            paint.setTextSize(120);
            SkPoint textPos = { 20, 110 };
            int len = 3;
            SkTextBlobBuilder textBlobBuilder;
            const SkTextBlobBuilder::RunBuffer& run = 
                    textBlobBuilder.allocRun(paint, len, textPos.fX, textPos.fY);
            run.glyphs[0] = 10;
            run.glyphs[1] = 20;
            run.glyphs[2] = 30;       
            sk_sp<const SkTextBlob> blob = textBlobBuilder.make();
            canvas->drawTextBlob(blob.get(), textPos.fX, textPos.fY, paint);
            SkScalar bounds[] = { 116, 134 };
            int count = paint.getTextBlobIntercepts(blob.get(), bounds, nullptr);
            std::vector<SkScalar> intervals;
            intervals.resize(count);
            (void) paint.getTextBlobIntercepts(blob.get(), bounds, &intervals.front());
            canvas->clear(SK_ColorWHITE);
            canvas->drawTextBlob(blob.get(), 0, 0, paint);
            paint.setColor(0xFFFF7777);
            SkScalar x = textPos.fX;
            for (int i = 0; i < count; i+= 2) {
                canvas->drawRect({x, bounds[0], intervals[i], bounds[1]}, paint);
                x = intervals[i + 1];
            }
            canvas->drawRect({intervals[count - 1], bounds[0], 180, bounds[1]}, paint);
}

#include "SkLightingImageFilter.h"

DEF_SIMPLE_GM(setimagefilter, canvas, 256, 256) {
        SkBitmap bitmap;
        bitmap.allocN32Pixels(100, 100);
        SkCanvas offscreen(bitmap);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorWHITE);
        paint.setTextSize(96);
        offscreen.clear(0);
        offscreen.drawString("e", 20, 70, paint);
        paint.setImageFilter(
               SkLightingImageFilter::MakePointLitDiffuse(SkPoint3::Make(80, 100, 10),
               SK_ColorWHITE, 1, 2, nullptr, nullptr));
        canvas->drawBitmap(bitmap, 0, 0, &paint);
}
