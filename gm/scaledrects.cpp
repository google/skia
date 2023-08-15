/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkString.h"

namespace skiagm {

// From crbug.com/1442854. Draws two rects (equivalent in device space) but which vary wildly
// in their sizes and scales. In both cases the clip is what is actually determining the
// final drawn geometry. For the red rectangle case the inverted skews were becoming very small and
// ran afoul of some logic in the DMSAA code that zeroed them out.
class ScaledRectsGM : public GM {
public:
    ScaledRectsGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("scaledrects"); }

    SkISize getISize() override { return SkISize::Make(128, 64); }

    void onDraw(SkCanvas* canvas) override {
        canvas->clipRect(SkRect::MakeXYWH(10, 50, 100, 10));

        {
            SkPaint blue;
            blue.setColor(SK_ColorBLUE);

            canvas->setMatrix(SkMatrix::MakeAll( 3.0f, -0.5f, 0.0f,
                                                -0.5f, -3.0f, 0.0f,
                                                 0.0f,  0.0f, 1.0f));

            canvas->drawRect(SkRect::MakeXYWH(-1000, -1000, 2000, 2000), blue);
        }

        {
            SkPaint red;
            red.setColor(SK_ColorRED);
            red.setBlendMode(SkBlendMode::kPlus);

            canvas->setMatrix(SkMatrix::MakeAll(3000.0f,  -500.0f, 0.0f,
                                                -500.0f, -3000.0f, 0.0f,
                                                   0.0f,     0.0f, 1.0f));

            canvas->drawRect(SkRect::MakeXYWH(-1, -1, 2, 2), red);
        }
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ScaledRectsGM;)

}  // namespace skiagm
