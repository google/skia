/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkBlendModePriv.h"
#include "SkPath.h"
#include "SkShader.h"

enum {
    kXfermodeCount = (int)SkBlendMode::kLastMode + 1 + 1,   // extra for arith
    kShapeSize = 22,
    kShapeSpacing = 36,
    kShapeTypeSpacing = 4 * kShapeSpacing / 3,
    kPaintSpacing = 4 * kShapeTypeSpacing,
    kLabelSpacing = 3 * kShapeSize,
    kMargin = kShapeSpacing / 2,
    kXfermodeTypeSpacing = kLabelSpacing + 2 * kPaintSpacing + kShapeTypeSpacing,
    kTitleSpacing = 3 * kShapeSpacing / 4,
    kSubtitleSpacing = 5 * kShapeSpacing / 8
};

constexpr SkColor kBGColor = 0xc8d2b887;

constexpr SkColor kShapeColors[2] = {
    0x82ff0080,   // input color unknown
    0xff00ffff,   // input color opaque
};

enum Shape {
    kSquare_Shape,
    kDiamond_Shape,
    kOval_Shape,
    kConcave_Shape,

    kLast_Shape = kConcave_Shape
};

/**
 * Verifies AA works properly on all Xfermodes, including arithmetic, with both opaque and unknown
 * src colors.
 */
class AAXfermodesGM : public skiagm::GM {
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
                             (1 + (int)SkBlendMode::kLastCoeffMode) * kShapeSpacing);
    }

    void onOnceBeforeDraw() override {
        fLabelPaint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&fLabelPaint);
        fLabelPaint.setTextSize(5 * kShapeSize/8);
        fLabelPaint.setSubpixelText(true);

        constexpr SkScalar radius = -1.4f * kShapeSize/2;
        SkPoint pts[4] = {
            {-radius, 0},
            {0, -1.33f * radius},
            {radius, 0},
            {0, 1.33f * radius}
        };
        fOval.moveTo(pts[0]);
        fOval.quadTo(pts[1], pts[2]);
        fOval.quadTo(pts[3], pts[0]);

        fConcave.moveTo(-radius, 0);
        fConcave.quadTo(0, 0, 0, -radius);
        fConcave.quadTo(0, 0, radius, 0);
        fConcave.quadTo(0, 0, 0, radius);
        fConcave.quadTo(0, 0, -radius, 0);
        fConcave.close();
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
            size_t firstMode = ((size_t)SkBlendMode::kLastCoeffMode + 1) * xfermodeSet;
            canvas->save();

            if (kShape_Pass == drawingPass) {
                fLabelPaint.setTextAlign(SkPaint::kCenter_Align);
                canvas->drawString("Src Unknown",
                        kLabelSpacing + kShapeTypeSpacing * 1.5f + kShapeSpacing / 2,
                        kSubtitleSpacing / 2 + fLabelPaint.getTextSize() / 3, fLabelPaint);
                canvas->drawString("Src Opaque",
                        kLabelSpacing + kShapeTypeSpacing * 1.5f + kShapeSpacing / 2 +
                        kPaintSpacing, kSubtitleSpacing / 2 + fLabelPaint.getTextSize() / 3,
                        fLabelPaint);
            }

            canvas->translate(0, kSubtitleSpacing + kShapeSpacing/2);

            for (size_t m = 0; m <= (size_t)SkBlendMode::kLastCoeffMode; m++) {
                if (firstMode + m > (size_t)SkBlendMode::kLastMode) {
                    break;
                }
                SkBlendMode mode = static_cast<SkBlendMode>(firstMode + m);
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
                                canvas->drawColor(kBGColor, SkBlendMode::kSrc);
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
        canvas->saveLayer(nullptr, nullptr);

        canvas->translate(kMargin, kMargin);
        draw_pass(canvas, kBackground_Pass);

        SkPaint titlePaint(fLabelPaint);
        titlePaint.setTextSize(9 * titlePaint.getTextSize() / 8);
        titlePaint.setFakeBoldText(true);
        titlePaint.setTextAlign(SkPaint::kCenter_Align);
        canvas->drawString("Porter Duff",
                         kLabelSpacing + 4 * kShapeTypeSpacing,
                         kTitleSpacing / 2 + titlePaint.getTextSize() / 3, titlePaint);
        canvas->drawString("Advanced",
                         kXfermodeTypeSpacing + kLabelSpacing + 4 * kShapeTypeSpacing,
                         kTitleSpacing / 2 + titlePaint.getTextSize() / 3, titlePaint);

        draw_pass(canvas, kShape_Pass);
        canvas->restore();
    }

    void drawModeName(SkCanvas* canvas, SkBlendMode mode) {
        const char* modeName = SkBlendMode_Name(mode);
        fLabelPaint.setTextAlign(SkPaint::kRight_Align);
        canvas->drawString(modeName, kLabelSpacing - kShapeSize / 4,
                         fLabelPaint.getTextSize() / 4, fLabelPaint);
    }

    void setupShapePaint(SkCanvas* canvas, GrColor color, SkBlendMode mode, SkPaint* paint) {
        paint->setColor(color);

        if (mode == SkBlendMode::kPlus) {
            // Check for overflow, otherwise we might get confusing AA artifacts.
            int maxSum = SkTMax(SkTMax(SkColorGetA(kBGColor) + SkColorGetA(color),
                                       SkColorGetR(kBGColor) + SkColorGetR(color)),
                                SkTMax(SkColorGetG(kBGColor) + SkColorGetG(color),
                                       SkColorGetB(kBGColor) + SkColorGetB(color)));

            if (maxSum > 255) {
                SkPaint dimPaint;
                dimPaint.setAntiAlias(false);
                dimPaint.setBlendMode(SkBlendMode::kDstIn);
                if (255 != paint->getAlpha()) {
                    // Dim the src and dst colors.
                    dimPaint.setARGB(255 * 255 / maxSum, 0, 0, 0);
                    paint->setAlpha(255 * paint->getAlpha() / maxSum);
                } else {
                    // Just clear the dst, we need to preserve the paint's opacity.
                    dimPaint.setARGB(0, 0, 0, 0);
                }
                canvas->drawRect({ -kShapeSpacing/2, -kShapeSpacing/2,
                                   kShapeSpacing/2 + 3 * kShapeTypeSpacing, kShapeSpacing/2 },
                                 dimPaint);
            }
        }
    }

    void drawShape(SkCanvas* canvas, Shape shape, const SkPaint& paint, SkBlendMode mode) {
        SkASSERT(mode <= SkBlendMode::kLastMode);
        SkPaint shapePaint(paint);
        shapePaint.setAntiAlias(kSquare_Shape != shape);
        shapePaint.setBlendMode(mode);

        switch (shape) {
            case kSquare_Shape:
                canvas->drawRect({ -kShapeSize/2, -kShapeSize/2, kShapeSize/2, kShapeSize/2 },
                                 shapePaint);
                break;

            case kDiamond_Shape:
                canvas->save();
                canvas->rotate(45);
                canvas->drawRect({ -kShapeSize/2, -kShapeSize/2, kShapeSize/2, kShapeSize/2 },
                                 shapePaint);
                canvas->restore();
                break;

            case kOval_Shape:
                canvas->save();
                canvas->rotate(static_cast<SkScalar>((511 * (int)mode + 257) % 360));
                canvas->drawPath(fOval, shapePaint);
                canvas->restore();
                break;

            case kConcave_Shape:
                canvas->drawPath(fConcave, shapePaint);
                break;

            default:
                SkFAIL("Invalid shape.");
        }
    }

private:
    SkPaint   fLabelPaint;
    SkPath    fOval;
    SkPath    fConcave;

    typedef skiagm::GM INHERITED;
};
DEF_GM( return new AAXfermodesGM; )
