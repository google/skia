
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkArithmeticMode.h"
#include "SkShader.h"
#include "SkXfermode.h"

enum {
    kXfermodeCount = SkXfermode::kLastMode + 2, // All xfermodes plus arithmetic mode.
    kShapeSize = 22,
    kShapeSpacing = 36,
    kShapeTypeSpacing = 4 * kShapeSpacing/3,
    kPaintSpacing = 2 * kShapeSpacing,
    kPadding = (kPaintSpacing - kShapeSpacing) / 2,

    kPaintWidth = 3*kShapeSpacing + 2*kShapeTypeSpacing + kShapeSize,
    kPaintPadding = kPaintSpacing - kShapeSize,
};

static const SkColor kBGColor = SkColorSetARGB(200, 210, 184, 135);

static const SkColor kShapeColors[3] = {
    SkColorSetARGB(130, 255, 0, 128),   // input color unknown
    SkColorSetARGB(255, 0, 255, 255),   // input color opaque
    SkColorSetARGB(255, 255, 255, 255)  // input solid white
};

enum Shape {
    kSquare_Shape,
    kDiamond_Shape,
    kOval_Shape,

    kLast_Shape = kOval_Shape
};

namespace skiagm {

static void draw_shape(SkCanvas* canvas, Shape shape, const SkColor color, size_t xfermodeIdx);

/**
 * Verifies AA works properly on all Xfermodes, including arithmetic, with various color invariants.
 */
class AAXfermodesGM : public GM {
public:
    AAXfermodesGM() {}

protected:
    SkString onShortName() override {
        return SkString("aaxfermodes");
    }

    SkISize onISize() override {
        return SkISize::Make(3*kPaintWidth + 2*kPaintPadding + 2*kPadding,
                             (2 + SkXfermode::kLastCoeffMode) * kShapeSpacing + 2*kPadding);
    }

    void onDraw(SkCanvas* canvas) override {
        sk_tool_utils::draw_checkerboard(canvas, 0xffffffff, 0xffc0c0c0, 10);

        canvas->saveLayer(NULL, NULL);
        canvas->drawColor(kBGColor, SkXfermode::kSrc_Mode);

        canvas->translate(kPadding + kShapeSize/2, kPadding + kShapeSpacing + kShapeSize/2);

        for (size_t colorIdx = 0; colorIdx < SK_ARRAY_COUNT(kShapeColors); colorIdx++) {
            SkColor color = kShapeColors[colorIdx];

            for (size_t shapeIdx = 0; shapeIdx <= kLast_Shape; shapeIdx++) {
                Shape shape = static_cast<Shape>(shapeIdx);
                canvas->save();

                for (size_t xfermodeIdx = 0; xfermodeIdx < kXfermodeCount; xfermodeIdx++) {
                    draw_shape(canvas, shape, color, xfermodeIdx);

                    if (xfermodeIdx == SkXfermode::kLastCoeffMode) {
                        // New column.
                        canvas->restore();
                        canvas->translate(kShapeSpacing, 0);
                        canvas->save();
                    } else {
                        canvas->translate(0, kShapeSpacing);
                    }
                }

                canvas->restore();

                if (shape != kLast_Shape) {
                    canvas->translate(kShapeTypeSpacing, 0);
                } else {
                    canvas->translate(kPaintSpacing, 0);
                }
            }
        }

        canvas->restore();

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&textPaint);
        textPaint.setTextAlign(SkPaint::kCenter_Align);
        textPaint.setFakeBoldText(true);
        textPaint.setTextSize(21 * kShapeSize/32);

        canvas->translate(kPadding + kPaintWidth/2,
                          kPadding + kShapeSize/2 + textPaint.getTextSize()/4);
        canvas->drawText("input color unknown", sizeof("input color unknown") - 1, 0, 0, textPaint);

        canvas->translate(kPaintWidth + kPaintPadding, 0);
        canvas->drawText("input color opaque", sizeof("input color opaque") - 1, 0, 0, textPaint);

        canvas->translate(kPaintWidth + kPaintPadding, 0);
        canvas->drawText("input solid white", sizeof("input solid white") - 1, 0, 0, textPaint);
    }

private:
    typedef GM INHERITED;
};

static void draw_shape(SkCanvas* canvas, Shape shape, const SkColor color, size_t xfermodeIdx) {
    SkPaint shapePaint;
    shapePaint.setAntiAlias(kSquare_Shape != shape);
    shapePaint.setColor(color);

    SkAutoTUnref<SkXfermode> xfermode;
    if (xfermodeIdx <= SkXfermode::kLastMode) {
        SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(xfermodeIdx);
        xfermode.reset(SkXfermode::Create(mode));
    } else {
        xfermode.reset(SkArithmeticMode::Create(+1.0f, +0.25f, -0.5f, +0.1f));
    }
    shapePaint.setXfermode(xfermode);

    if (xfermodeIdx == SkXfermode::kPlus_Mode) {
        // Check for overflow and dim the src and dst colors if we need to, otherwise we might get
        // confusing AA artifacts.
        int maxSum = SkTMax(SkTMax(SkColorGetA(kBGColor) + SkColorGetA(color),
                                   SkColorGetR(kBGColor) + SkColorGetR(color)),
                            SkTMax(SkColorGetG(kBGColor) + SkColorGetG(color),
                                   SkColorGetB(kBGColor) + SkColorGetB(color)));

        if (maxSum > 255) {
            SkPaint dimPaint;
            dimPaint.setARGB(255 * 255 / maxSum, 0, 0, 0);
            dimPaint.setAntiAlias(false);
            dimPaint.setXfermode(SkXfermode::Create(SkXfermode::kDstIn_Mode));
            canvas->drawRectCoords(-kShapeSpacing/2, -kShapeSpacing/2,
                                   kShapeSpacing/2, kShapeSpacing/2, dimPaint);

            shapePaint.setAlpha(255 * shapePaint.getAlpha() / maxSum);
        }
    }

    switch (shape) {
        case kSquare_Shape:
            canvas->drawRectCoords(-kShapeSize/2, -kShapeSize/2, kShapeSize/2, kShapeSize/2,
                                   shapePaint);
            break;

        case kDiamond_Shape:
            canvas->save();
            canvas->rotate(45);
            canvas->drawRectCoords(-kShapeSize/2, -kShapeSize/2, kShapeSize/2, kShapeSize/2,
                                   shapePaint);
            canvas->restore();
            break;

        case kOval_Shape:
            canvas->save();
            canvas->rotate(static_cast<SkScalar>((511 * xfermodeIdx + 257) % 360));
            canvas->drawArc(SkRect::MakeLTRB(-kShapeSize/2, -1.4f * kShapeSize/2,
                                             kShapeSize/2, 1.4f * kShapeSize/2),
                            0, 360, true, shapePaint);
            canvas->restore();
            break;

        default:
            SkFAIL("Invalid shape.");
    }
}

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new AAXfermodesGM; }
static GMRegistry reg(MyFactory);

}
