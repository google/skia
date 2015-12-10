/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkLayerDrawLooper.h"
#include "SkPath.h"
#include "SkRandom.h"

static SkRect inset(const SkRect& r) {
    SkRect rect = r;
    rect.inset(r.width() / 8, r.height() / 8);
    return rect;
}

class PathInteriorGM : public skiagm::GM {
public:
    PathInteriorGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFDDDDDD));
    }

protected:
    SkISize onISize() override {
        return SkISize::Make(770, 770);
    }

    SkString onShortName() override {
        return SkString("pathinterior");
    }

    void show(SkCanvas* canvas, const SkPath& path) {
        SkPaint paint;
        paint.setAntiAlias(true);

        SkRect rect;
#if 0
        bool hasInterior = path.hasRectangularInterior(&rect);
#else
        bool hasInterior = false;
#endif

        paint.setColor(sk_tool_utils::color_to_565(hasInterior ? 0xFF8888FF : SK_ColorGRAY));
        canvas->drawPath(path, paint);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(path, paint);

        if (hasInterior) {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(0x8800FF00);
            canvas->drawRect(rect, paint);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(8.5f, 8.5f);

        const SkRect rect = { 0, 0, 80, 80 };
        const SkScalar RAD = rect.width()/8;

        int i = 0;
        for (int insetFirst = 0; insetFirst <= 1; ++insetFirst) {
            for (int doEvenOdd = 0; doEvenOdd <= 1; ++doEvenOdd) {
                for (int outerRR = 0; outerRR <= 1; ++outerRR) {
                    for (int innerRR = 0; innerRR <= 1; ++innerRR) {
                        for (int outerCW = 0; outerCW <= 1; ++outerCW) {
                            for (int innerCW = 0; innerCW <= 1; ++innerCW) {
                                SkPath path;
                                path.setFillType(doEvenOdd ? SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType);
                                SkPath::Direction outerDir = outerCW ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
                                SkPath::Direction innerDir = innerCW ? SkPath::kCW_Direction : SkPath::kCCW_Direction;

                                SkRect r = insetFirst ? inset(rect) : rect;
                                if (outerRR) {
                                    path.addRoundRect(r, RAD, RAD, outerDir);
                                } else {
                                    path.addRect(r, outerDir);
                                }
                                r = insetFirst ? rect : inset(rect);
                                if (innerRR) {
                                    path.addRoundRect(r, RAD, RAD, innerDir);
                                } else {
                                    path.addRect(r, innerDir);
                                }

                                SkScalar dx = (i / 8) * rect.width() * 6 / 5;
                                SkScalar dy = (i % 8) * rect.height() * 6 / 5;
                                i++;
                                path.offset(dx, dy);

                                this->show(canvas, path);
                            }
                        }
                    }
                }
            }
        }
    }

private:

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PathInteriorGM; )
