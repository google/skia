/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <utility>

namespace skiagm {

constexpr SkColor gPathColor = SK_ColorYELLOW;

class ComplexClip3GM : public GM {
public:
    ComplexClip3GM(bool doSimpleClipFirst)
        : fDoSimpleClipFirst(doSimpleClipFirst) {
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString getName() const override {
        SkString str;
        str.printf("complexclip3_%s", fDoSimpleClipFirst ? "simple" : "complex");
        return str;
    }

    SkISize getISize() override { return SkISize::Make(400, 950); }

    void onDraw(SkCanvas* canvas) override {
        SkPath clipSimple = SkPath::Circle(70, 50, 20);

        SkRect r1 = { 10, 20, 70, 80 };
        SkPath clipComplex = SkPathBuilder().moveTo(40,  50)
                                            .arcTo(r1, 30, 300, false)
                                            .close()
                                            .detach();

        SkPath* firstClip = &clipSimple;
        SkPath* secondClip = &clipComplex;
        if (!fDoSimpleClipFirst) {
            std::swap(firstClip, secondClip);
        }

        SkPaint paint;
        paint.setAntiAlias(true);

        SkFont font(ToolUtils::DefaultPortableTypeface(), 20);

        constexpr struct {
            SkClipOp    fOp;
            const char* fName;
        } gOps[] = {
            {SkClipOp::kIntersect,         "I"},
            {SkClipOp::kDifference,        "D" },
        };

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));
        canvas->scale(3 * SK_Scalar1 / 4, 3 * SK_Scalar1 / 4);

        SkPaint pathPaint;
        pathPaint.setAntiAlias(true);
        pathPaint.setColor(gPathColor);

        for (int invA = 0; invA < 2; ++invA) {
            for (int aaBits = 0; aaBits < 4; ++aaBits) {
                canvas->save();
                for (size_t op = 0; op < std::size(gOps); ++op) {
                    for (int invB = 0; invB < 2; ++invB) {
                        bool doAAA = SkToBool(aaBits & 1);
                        bool doAAB = SkToBool(aaBits & 2);
                        bool doInvA = SkToBool(invA);
                        bool doInvB = SkToBool(invB);
                        canvas->save();
                        // set clip
                        firstClip->setFillType(doInvA ? SkPathFillType::kInverseEvenOdd :
                                               SkPathFillType::kEvenOdd);
                        secondClip->setFillType(doInvB ? SkPathFillType::kInverseEvenOdd :
                                                SkPathFillType::kEvenOdd);
                        canvas->clipPath(*firstClip, doAAA);
                        canvas->clipPath(*secondClip, gOps[op].fOp, doAAB);

                        // draw rect clipped
                        SkRect r = { 0, 0, 100, 100 };
                        canvas->drawRect(r, pathPaint);
                        canvas->restore();


                        SkScalar txtX = SkIntToScalar(10);
                        paint.setColor(SK_ColorBLACK);
                        SkString str;
                        str.printf("%s%s %s %s%s", doAAA ? "A" : "B",
                                                   doInvA ? "I" : "N",
                                                   gOps[op].fName,
                                                   doAAB ? "A" : "B",
                                                   doInvB ? "I" : "N");

                        canvas->drawString(str.c_str(), txtX, SkIntToScalar(130), font, paint);
                        if (doInvB) {
                            canvas->translate(SkIntToScalar(150),0);
                        } else {
                            canvas->translate(SkIntToScalar(120),0);
                        }
                    }
                }
                canvas->restore();
                canvas->translate(0, SkIntToScalar(150));
            }
        }
    }

private:
    bool fDoSimpleClipFirst;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

// Simple clip first
DEF_GM( return new ComplexClip3GM(true); )
// Complex clip first
DEF_GM( return new ComplexClip3GM(false); )
}  // namespace skiagm
