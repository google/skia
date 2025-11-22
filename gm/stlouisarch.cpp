/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/private/base/SkTArray.h"

using namespace skia_private;

namespace skiagm {

// this GM tests hairlines which fill nearly the entire render target
class StLouisArchGM : public GM {
protected:
    SkString getName() const override { return SkString("stlouisarch"); }

    SkISize getISize() override { return SkISize::Make((int)kWidth, (int)kHeight); }

    void onOnceBeforeDraw() override {
        {
            SkPath bigQuad = SkPathBuilder()
                             .moveTo(0, 0)
                             .quadTo(kWidth/2, kHeight, kWidth, 0)
                             .detach();
            fPaths.push_back(bigQuad);
        }

        {
            SkScalar yPos = kHeight / 2 + 10;
            SkPath degenBigQuad = SkPathBuilder()
                                  .moveTo(0, yPos)
                                  .quadTo(0, yPos, kWidth, yPos)
                                  .detach();
            fPaths.push_back(degenBigQuad);
        }

        {
            SkPath bigCubic = SkPathBuilder()
                              .moveTo(0, 0)
                              .cubicTo(0, kHeight, kWidth, kHeight, kWidth, 0)
                              .detach();
            fPaths.push_back(bigCubic);
        }

        {
            SkScalar yPos = kHeight / 2;
            SkPath degenBigCubic = SkPathBuilder()
                                   .moveTo(0, yPos)
                                   .cubicTo(0, yPos, 0, yPos, kWidth, yPos)
                                   .detach();
            fPaths.push_back(degenBigCubic);
        }

        {
            SkPath bigConic = SkPathBuilder()
                              .moveTo(0, 0)
                              .conicTo(kWidth/2, kHeight, kWidth, 0, .5)
                              .detach();
            fPaths.push_back(bigConic);
        }

        {
            SkScalar yPos = kHeight / 2 - 10;
            SkPath degenBigConic = SkPathBuilder()
                                   .moveTo(0, yPos)
                                   .conicTo(0, yPos, kWidth, yPos, .5)
                                   .detach();
            fPaths.push_back(degenBigConic);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->save();
        canvas->scale(1, -1);
        canvas->translate(0, -kHeight);
        for (int p = 0; p < fPaths.size(); ++p) {
            SkPaint paint;
            paint.setARGB(0xff, 0, 0, 0);
            paint.setAntiAlias(true);
            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(0);
            canvas->drawPath(fPaths[p], paint);
        }
        canvas->restore();
    }

    const SkScalar kWidth = 256;
    const SkScalar kHeight = 256;

private:
    TArray<SkPath> fPaths;
    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new StLouisArchGM; )

}  // namespace skiagm
