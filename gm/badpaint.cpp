/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkShader.h"

/** This GM draws with invalid paints. It should draw nothing other than the background. */
class BadPaintGM : public skiagm::GM {
 public:
    BadPaintGM() {}

protected:
    SkString onShortName() override { return SkString("badpaint"); }

    SkISize onISize() override { return SkISize::Make(100, 100); }

    void onOnceBeforeDraw() override {
        SkBitmap emptyBmp;

        SkBitmap blueBmp;
        blueBmp.allocN32Pixels(10, 10);
        blueBmp.eraseColor(SK_ColorBLUE);

        SkMatrix badMatrix;
        badMatrix.setAll(0, 0, 0, 0, 0, 0, 0, 0, 0);

        // Empty bitmap.
        fPaints.push_back().setColor(SK_ColorGREEN);
        fPaints.back().setShader(emptyBmp.makeShader());

        // Non-invertible local matrix.
        fPaints.push_back().setColor(SK_ColorGREEN);
        fPaints.back().setShader(blueBmp.makeShader(&badMatrix));
    }

    void onDraw(SkCanvas* canvas) override {
        SkRect rect = SkRect::MakeXYWH(10, 10, 80, 80);
        for (int i = 0; i < fPaints.count(); ++i) {
            canvas->drawRect(rect, fPaints[i]);
        }
    }

private:
    SkTArray<SkPaint> fPaints;

    typedef skiagm::GM INHERITED;
};

/////////////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BadPaintGM;)
