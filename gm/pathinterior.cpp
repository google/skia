/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkLayerDrawLooper.h"
#include "SkBlurMaskFilter.h"

static SkRect inset(const SkRect& r) {
    SkRect rect = r;
    rect.inset(r.width() / 8, r.height() / 8);
    return rect;
}

class PathInteriorGM : public skiagm::GM {
public:
    PathInteriorGM() {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    virtual SkISize onISize() {
        return SkISize::Make(960, 480);
    }

    virtual SkString onShortName() SK_OVERRIDE {
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

        paint.setColor(hasInterior ? 0xFF8888FF : SK_ColorGRAY);
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

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->translate(8, 8);

        const SkRect rect = { 0, 0, 100, 100 };
        const SkScalar RAD = rect.width()/8;

        int i = 0;
        for (int doEvenOdd = 0; doEvenOdd <= 1; ++doEvenOdd) {
            for (int outerRR = 0; outerRR <= 1; ++outerRR) {
                for (int innerRR = 0; innerRR <= 1; ++innerRR) {
                    for (int outerCW = 0; outerCW <= 1; ++outerCW) {
                        for (int innerCW = 0; innerCW <= 1; ++innerCW) {
                            SkPath path;
                            path.setFillType(doEvenOdd ? SkPath::kEvenOdd_FillType : SkPath::kWinding_FillType);
                            SkPath::Direction outerDir = outerCW ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
                            SkPath::Direction innerDir = innerCW ? SkPath::kCW_Direction : SkPath::kCCW_Direction;
                            if (outerRR) {
                                path.addRoundRect(rect, RAD, RAD, outerDir);
                            } else {
                                path.addRect(rect, outerDir);
                            }
                            SkRect inner = inset(rect);
                            if (innerRR) {
                                path.addRoundRect(inner, RAD, RAD, innerDir);
                            } else {
                                path.addRect(inner, innerDir);
                            }

                            SkScalar dx = (i / 4) * rect.width() * 6 / 5;
                            SkScalar dy = (i % 4) * rect.height() * 6 / 5;
                            i++;
                            path.offset(dx, dy);

                            this->show(canvas, path);
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

static skiagm::GM* MyFactory(void*) { return new PathInteriorGM; }
static skiagm::GMRegistry reg(MyFactory);

