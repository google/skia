/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkClipOpPriv.h"

// This GM tests 3 different ways of drawing four shadows around a square:
//      just using 4 blurred rects
//      using 4 1-level draw loopers
//      using 1 4-level draw looper
// They all produce exactly the same pixels
class MegaLooperGM : public skiagm::GM {
public:
    // The types define "<# of loopers> x <# of stages per looper>"
    enum Type {
        k0x0_Type,  // draw without loopers at all
        k4x1_Type,  // a looper for each shadow
        k1x4_Type,  // all four shadows in one looper
    };

    MegaLooperGM(Type type) : fType(type) {}

protected:
    virtual SkString onShortName() {
        switch (fType) {
        case k0x0_Type:
            return SkString("megalooper_0x0");
            break;
        case k4x1_Type:
            return SkString("megalooper_4x1");
            break;
        case k1x4_Type:     // fall through
        default:
            return SkString("megalooper_1x4");
            break;
        }
    }

    virtual SkISize onISize() {
        return SkISize::Make(kWidth, kHeight);
    }

    virtual void onDraw(SkCanvas* canvas) {
        for (int y = 100; y < kHeight; y += 200) {
            for (int x = 100; x < kWidth; x += 200) {
                switch (fType) {
                    case k0x0_Type:
                        draw0x0(canvas, SkIntToScalar(x), SkIntToScalar(y));
                        break;
                    case k4x1_Type:
                        draw4x1(canvas, SkIntToScalar(x), SkIntToScalar(y));
                        break;
                    case k1x4_Type:     // fall through
                    default:
                        draw1x4(canvas, SkIntToScalar(x), SkIntToScalar(y));
                        break;
                }
            }
        }
    }

private:
    static constexpr int kWidth = 800;
    static constexpr int kHeight = 800;
    static constexpr int kHalfOuterClipSize = 100;
    static constexpr int kHalfSquareSize = 50;
    static constexpr int kOffsetToOutsideClip = kHalfSquareSize + kHalfOuterClipSize + 1;

    static const SkPoint gBlurOffsets[4];
    static const SkColor gColors[4];
    Type fType;

    // Just draw a blurred rect at each of the four corners of a square (centered at x,y).
    // Use two clips to define a rectori where we want pixels to appear.
    void draw0x0(SkCanvas* canvas, SkScalar x, SkScalar y) {
        SkRect innerClip = { -kHalfSquareSize, -kHalfSquareSize, kHalfSquareSize, kHalfSquareSize };
        innerClip.offset(x, y);

        SkRect outerClip = {
            -kHalfOuterClipSize-kHalfSquareSize, -kHalfOuterClipSize-kHalfSquareSize,
             kHalfOuterClipSize+kHalfSquareSize,  kHalfOuterClipSize+kHalfSquareSize
        };
        outerClip.offset(x, y);

        canvas->save();
        canvas->clipRect(outerClip, kIntersect_SkClipOp);
        canvas->clipRect(innerClip, kDifference_SkClipOp);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setMaskFilter(MakeBlur());

        for (int i = 0; i < 4; ++i) {
            paint.setColor(gColors[i]);

            SkRect rect = { -kHalfSquareSize, -kHalfSquareSize, kHalfSquareSize, kHalfSquareSize };
            rect.offset(gBlurOffsets[i]);
            rect.offset(x, y);
            canvas->drawRect(rect, paint);
        }

        canvas->restore();
    }

    static sk_sp<SkMaskFilter> MakeBlur() {
        const SkScalar kBlurSigma = SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(25));

        return SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, kBlurSigma);
    }

    // This draws 4 blurred shadows around a single square (centered at x, y).
    // Each blur is offset +/- half the square's side in x & y from the original
    // (so each blurred rect is centered at one of the corners of the original).
    // For each blur a large outer clip is centered around the blurred rect
    // while a difference clip stays at the location of the original rect.
    // Each blurred rect is drawn with a draw looper where the original (non-
    // blurred rect) is offset to reside outside of the large outer clip (so
    // it never appears) but the offset in the draw looper is used to translate
    // the blurred version back into the clip.
    void draw4x1(SkCanvas* canvas, SkScalar x, SkScalar y) {

        for (int i = 0; i < 4; ++i) {
            SkPaint loopPaint;

            loopPaint.setLooper(create1Looper(-kOffsetToOutsideClip, 0, gColors[i]));
            loopPaint.setAntiAlias(true);

            SkRect outerClip = {
                -kHalfOuterClipSize, -kHalfOuterClipSize,
                kHalfOuterClipSize, kHalfOuterClipSize
            };
            outerClip.offset(x, y);
            // center it on the blurred rect
            outerClip.offset(gBlurOffsets[i]);

            SkRect rect = { -kHalfSquareSize, -kHalfSquareSize, kHalfSquareSize, kHalfSquareSize };
            rect.offset(x, y);

            canvas->save();
                canvas->clipRect(outerClip, kIntersect_SkClipOp);
                canvas->clipRect(rect, kDifference_SkClipOp);

                // move the rect to where we want the blur to appear
                rect.offset(gBlurOffsets[i]);
                // then move it outside the clip (the blur stage of the draw
                // looper will undo this translation)
                rect.offset(SkIntToScalar(kOffsetToOutsideClip), 0);

                canvas->drawRect(rect, loopPaint);
            canvas->restore();
        }
    }

    // Create a 1-tier drawlooper
    sk_sp<SkDrawLooper> create1Looper(SkScalar xOff, SkScalar yOff, SkColor color) {
        SkLayerDrawLooper::Builder looperBuilder;
        SkLayerDrawLooper::LayerInfo info;

        info.fPaintBits = SkLayerDrawLooper::kColorFilter_Bit |
                          SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkBlendMode::kSrc;
        info.fOffset.set(xOff, yOff);
        info.fPostTranslate = false;

        SkPaint* paint = looperBuilder.addLayer(info);

        paint->setMaskFilter(MakeBlur());

        paint->setColorFilter(SkColorFilters::Blend(color, SkBlendMode::kSrcIn));

        return looperBuilder.detach();
    }

    void draw1x4(SkCanvas* canvas, SkScalar x, SkScalar y) {
        SkRect rect = { -kHalfSquareSize, -kHalfSquareSize, kHalfSquareSize, kHalfSquareSize };
        rect.offset(x, y);

        SkRect outerClip = {
            -kHalfOuterClipSize-kHalfSquareSize, -kHalfOuterClipSize-kHalfSquareSize,
             kHalfOuterClipSize+kHalfSquareSize,  kHalfOuterClipSize+kHalfSquareSize
        };
        outerClip.offset(x, y);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLooper(create4Looper(-kOffsetToOutsideClip-kHalfSquareSize, 0));

        canvas->save();
            canvas->clipRect(outerClip, kIntersect_SkClipOp);
            canvas->clipRect(rect, kDifference_SkClipOp);

            rect.offset(SkIntToScalar(kOffsetToOutsideClip+kHalfSquareSize), 0);
            canvas->drawRect(rect, paint);
        canvas->restore();
    }

    // Create a 4-tier draw looper
    sk_sp<SkDrawLooper> create4Looper(SkScalar xOff, SkScalar yOff) {
        SkLayerDrawLooper::Builder looperBuilder;
        SkLayerDrawLooper::LayerInfo info;

        info.fPaintBits = SkLayerDrawLooper::kColorFilter_Bit |
                          SkLayerDrawLooper::kMaskFilter_Bit;
        info.fColorMode = SkBlendMode::kSrc;
        info.fPostTranslate = false;

        SkPaint* paint;

        for (int i = 3; i >= 0; --i) {
            info.fOffset.set(xOff+gBlurOffsets[i].fX, yOff+gBlurOffsets[i].fY);
            paint = looperBuilder.addLayer(info);

            paint->setMaskFilter(MakeBlur());

            paint->setColorFilter(SkColorFilters::Blend(gColors[i], SkBlendMode::kSrcIn));
        }

        return looperBuilder.detach();
    }

    typedef GM INHERITED;
};

const SkPoint MegaLooperGM::gBlurOffsets[4] = {
    {  kHalfSquareSize,  kHalfSquareSize },
    { -kHalfSquareSize,  kHalfSquareSize },
    {  kHalfSquareSize, -kHalfSquareSize },
    { -kHalfSquareSize, -kHalfSquareSize }
};

const SkColor MegaLooperGM::gColors[4] = {
    SK_ColorGREEN, SK_ColorYELLOW, SK_ColorBLUE, SK_ColorRED
};

DEF_GM( return new MegaLooperGM(MegaLooperGM::k0x0_Type); )
DEF_GM( return new MegaLooperGM(MegaLooperGM::k4x1_Type); )
DEF_GM( return new MegaLooperGM(MegaLooperGM::k1x4_Type); )
