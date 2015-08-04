
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkArithmeticMode.h"
#include "SkPath.h"
#include "SkShader.h"
#include "SkXfermode.h"

enum {
    kXfermodeCount = SkXfermode::kLastMode + 2, // All xfermodes plus arithmetic mode.
    kShapeSize = 22,
    kShapeSpacing = 36,
    kShapeTypeSpacing = 4 * kShapeSpacing / 3,
    kPaintSpacing = 3 * kShapeTypeSpacing,
    kLabelSpacing = 3 * kShapeSize,
    kMargin = kShapeSpacing / 2,
    kXfermodeTypeSpacing = kLabelSpacing + 2 * kPaintSpacing + kShapeTypeSpacing,
    kTitleSpacing = 3 * kShapeSpacing / 4,
    kSubtitleSpacing = 5 * kShapeSpacing / 8
};

static const SkColor kBGColor = SkColorSetARGB(200, 210, 184, 135);

static const SkColor kShapeColors[2] = {
    SkColorSetARGB(130, 255, 0, 128),   // input color unknown
    SkColorSetARGB(255, 0, 255, 255)   // input color opaque
};

enum Shape {
    kSquare_Shape,
    kDiamond_Shape,
    kOval_Shape,

    kLast_Shape = kOval_Shape
};

namespace skiagm {

/**
 * Verifies AA works properly on all Xfermodes, including arithmetic, with both opaque and unknown
 * src colors.
 */
class AAXfermodesGM : public GM {
public:
    AAXfermodesGM() {}

protected:
    enum DrawingPass {
        kCheckerboard_Pass,
        kBackground_Pass,
        kShape_Pass
    };

    SkString onShortName() override {
        return SkString("aaxfermodes");
    }

    SkISize onISize() override {
        return SkISize::Make(2 * kMargin + 2 * kXfermodeTypeSpacing -
                             (kXfermodeTypeSpacing - (kLabelSpacing + 2 * kPaintSpacing)),
                             2 * kMargin + kTitleSpacing + kSubtitleSpacing +
                             (1 + SkXfermode::kLastCoeffMode) * kShapeSpacing);
    }

    void onOnceBeforeDraw() override {
        fLabelPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&fLabelPaint);
        fLabelPaint.setTextSize(5 * kShapeSize/8);
        fLabelPaint.setSubpixelText(true);

        static const SkScalar radius = -1.4f * kShapeSize/2;
        SkPoint pts[4] = {
            {-radius, 0},
            {0, -1.33f * radius},
            {radius, 0},
            {0, 1.33f * radius}
        };
        fPath.moveTo(pts[0]);
        fPath.quadTo(pts[1], pts[2]);
        fPath.quadTo(pts[3], pts[0]);
    }

    void draw_pass(SkCanvas* canvas, DrawingPass drawingPass) {
        SkRect clipRect =
                { -kShapeSize*11/16, -kShapeSize*11/16, kShapeSize*11/16, kShapeSize*11/16 };

        canvas->save();
        if (kCheckerboard_Pass == drawingPass) {
            canvas->translate(kMargin, kMargin);
        }
        canvas->translate(0, kTitleSpacing);

        for (size_t xfermodeSet = 0; xfermodeSet < 2; xfermodeSet++) {
            size_t firstMode = (SkXfermode::kLastCoeffMode + 1) * xfermodeSet;
            canvas->save();

            if (kShape_Pass == drawingPass) {
                fLabelPaint.setTextAlign(SkPaint::kCenter_Align);
                canvas->drawText("Src Unknown", sizeof("Src Unknown") - 1,
                        kLabelSpacing + kShapeSpacing / 2 + kShapeTypeSpacing,
                        kSubtitleSpacing / 2 + fLabelPaint.getTextSize() / 3, fLabelPaint);
                canvas->drawText("Src Opaque", sizeof("Src Opaque") - 1,
                        kLabelSpacing + kShapeSpacing / 2 + kShapeTypeSpacing + kPaintSpacing,
                        kSubtitleSpacing / 2 + fLabelPaint.getTextSize() / 3, fLabelPaint);
            }

            canvas->translate(0, kSubtitleSpacing + kShapeSpacing/2);

            for (size_t m = 0; m <= SkXfermode::kLastCoeffMode; m++) {
                SkXfermode::Mode mode = static_cast<SkXfermode::Mode>(firstMode + m);
                canvas->save();

                if (kShape_Pass == drawingPass) {
                    this->drawModeName(canvas, mode);
                }
                canvas->translate(kLabelSpacing + kShapeSpacing/2, 0);

                for (size_t colorIdx = 0; colorIdx < SK_ARRAY_COUNT(kShapeColors); colorIdx++) {
                    SkPaint paint;
                    this->setupShapePaint(canvas, kShapeColors[colorIdx], mode, &paint);
                    SkASSERT(colorIdx == 0 || 255 == paint.getAlpha());
                    canvas->save();

                    for (size_t shapeIdx = 0; shapeIdx <= kLast_Shape; shapeIdx++) {
                        if (kShape_Pass != drawingPass) {
                            canvas->save();
                            canvas->clipRect(clipRect);
                            if (kCheckerboard_Pass == drawingPass) {
                                sk_tool_utils::draw_checkerboard(canvas, 0xffffffff, 0xffc6c3c6,
                                        10);
                            } else {
                                SkASSERT(kBackground_Pass == drawingPass);
                                canvas->drawColor(kBGColor, SkXfermode::kSrc_Mode);
                            }
                            canvas->restore();
                        } else {
                            this->drawShape(canvas, static_cast<Shape>(shapeIdx), paint, mode);
                        }
                        canvas->translate(kShapeTypeSpacing, 0);
                    }

                    canvas->restore();
                    canvas->translate(kPaintSpacing, 0);
                }

                canvas->restore();
                canvas->translate(0, kShapeSpacing);
            }

            canvas->restore();
            canvas->translate(kXfermodeTypeSpacing, 0);
        }

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        draw_pass(canvas, kCheckerboard_Pass);
        canvas->saveLayer(NULL, NULL);

        canvas->translate(kMargin, kMargin);
        draw_pass(canvas, kBackground_Pass);

        SkPaint titlePaint(fLabelPaint);
        titlePaint.setTextSize(9 * titlePaint.getTextSize() / 8);
        titlePaint.setFakeBoldText(true);
        titlePaint.setTextAlign(SkPaint::kCenter_Align);
        canvas->drawText("Porter Duff", sizeof("Porter Duff") - 1,
                         kLabelSpacing + 3 * kShapeTypeSpacing,
                         kTitleSpacing / 2 + titlePaint.getTextSize() / 3, titlePaint);
        canvas->drawText("Advanced", sizeof("Advanced") - 1,
                         kXfermodeTypeSpacing + kLabelSpacing + 3 * kShapeTypeSpacing,
                         kTitleSpacing / 2 + titlePaint.getTextSize() / 3, titlePaint);

        draw_pass(canvas, kShape_Pass);
        canvas->restore();
    }

    void drawModeName(SkCanvas* canvas, SkXfermode::Mode mode) {
        const char* modeName = mode <= SkXfermode::kLastMode ? SkXfermode::ModeName(mode)
                                                             : "Arithmetic";
        fLabelPaint.setTextAlign(SkPaint::kRight_Align);
        canvas->drawText(modeName, strlen(modeName), kLabelSpacing - kShapeSize / 4,
                         fLabelPaint.getTextSize() / 3, fLabelPaint);
    }

    void setupShapePaint(SkCanvas* canvas, GrColor color, SkXfermode::Mode mode, SkPaint* paint) {
        paint->setColor(color);

        if (mode == SkXfermode::kPlus_Mode) {
            // Check for overflow, otherwise we might get confusing AA artifacts.
            int maxSum = SkTMax(SkTMax(SkColorGetA(kBGColor) + SkColorGetA(color),
                                       SkColorGetR(kBGColor) + SkColorGetR(color)),
                                SkTMax(SkColorGetG(kBGColor) + SkColorGetG(color),
                                       SkColorGetB(kBGColor) + SkColorGetB(color)));

            if (maxSum > 255) {
                SkPaint dimPaint;
                dimPaint.setAntiAlias(false);
                dimPaint.setXfermode(SkXfermode::Create(SkXfermode::kDstIn_Mode));
                if (255 != paint->getAlpha()) {
                    // Dim the src and dst colors.
                    dimPaint.setARGB(255 * 255 / maxSum, 0, 0, 0);
                    paint->setAlpha(255 * paint->getAlpha() / maxSum);
                } else {
                    // Just clear the dst, we need to preserve the paint's opacity.
                    dimPaint.setARGB(0, 0, 0, 0);
                }
                canvas->drawRectCoords(-kShapeSpacing/2, -kShapeSpacing/2,
                                       kShapeSpacing/2 + 2 * kShapeTypeSpacing,
                                       kShapeSpacing/2, dimPaint);
            }
        }
    }

    void drawShape(SkCanvas* canvas, Shape shape, const SkPaint& paint, SkXfermode::Mode mode) {
        SkPaint shapePaint(paint);
        shapePaint.setAntiAlias(kSquare_Shape != shape);

        SkAutoTUnref<SkXfermode> xfermode;
        if (mode <= SkXfermode::kLastMode) {
            xfermode.reset(SkXfermode::Create(mode));
        } else {
            xfermode.reset(SkArithmeticMode::Create(+1.0f, +0.25f, -0.5f, +0.1f));
        }
        shapePaint.setXfermode(xfermode);

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
                canvas->rotate(static_cast<SkScalar>((511 * mode + 257) % 360));
                canvas->drawPath(fPath, shapePaint);
                canvas->restore();
                break;

            default:
                SkFAIL("Invalid shape.");
        }
    }

private:
    SkPaint   fLabelPaint;
    SkPath    fPath;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new AAXfermodesGM; }
static GMRegistry reg(MyFactory);

}
