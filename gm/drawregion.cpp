/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkRegion.h"

/**
 *  This is very similar to the RectGrid macrobench in Android.
 */
class DrawRegionGM : public skiagm::GM {
public:
    DrawRegionGM() {}

protected:
    SkString onShortName() override {
        return SkString("drawregion");
    }

    SkISize onISize() override {
        return SkISize::Make(500, 500);
    }

    bool runAsBench() const override {
        return true;
    }

    void onOnceBeforeDraw() override {
        for (int x = 50; x < 250; x+=2) {
            for (int y = 50; y < 250; y+=2) {
                fRegion.op(x, y, x + 1, y + 1, SkRegion::kUnion_Op);
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 10);

        SkPaint paint;
        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(0xFFFF00FF);
        canvas->drawRect(SkRect::MakeLTRB(50.0f, 50.0f, 250.0f, 250.0f), paint);

        paint.setColor(0xFF00FFFF);
        canvas->drawRegion(fRegion, paint);
    }

    SkRegion fRegion;

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new DrawRegionGM; )
