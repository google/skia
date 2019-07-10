/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

#include "src/core/SkMathPriv.h"

uint32_t TestingOnly_Make_Approx(uint32_t value);

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 1000);
    }

    void check(uint32_t val, uint32_t floorPow2, uint32_t ceilPow2, uint32_t expectedApprox,
               bool valIsPow2) {
        SkASSERT(SkIsPow2(val) == valIsPow2);

        uint32_t low = GrPrevPow2(val);
        SkASSERT(SkIsPow2(low));

        uint32_t high = GrNextPow2(val);
        SkASSERT(SkIsPow2(high));

        uint32_t approx = TestingOnly_Make_Approx(val);
        SkASSERT(0 == (approx % 2));

        SkDebugf("%u %u (%u) %u (%u) %u (%u)\n", val,
            low, floorPow2,
            high, ceilPow2,
            approx, expectedApprox);

        if (valIsPow2) {
            SkASSERT(low == val);
            SkASSERT(high == val);
            SkASSERT(approx == val);
        } else {
            SkASSERT(low == floorPow2);
            SkASSERT(high == ceilPow2);
            SkASSERT(approx == expectedApprox);
        }
    }

    void onDraw(SkCanvas* canvas) override {

        uint32_t prevPowerOf2 = 0, curPowerOf2 = 1;
        for (int exp = 0; exp < 5; ++exp, prevPowerOf2 = curPowerOf2) {
            curPowerOf2 = 1 << exp;

            this->check(curPowerOf2-1, prevPowerOf2, curPowerOf2, 16, curPowerOf2 == 2);
            this->check(curPowerOf2, prevPowerOf2, curPowerOf2, 16, true);
            this->check(curPowerOf2+1, prevPowerOf2, curPowerOf2, 16, curPowerOf2 <= 2);
        }

        struct {
            uint32_t fVal;
            uint32_t fFloorPow2;
            uint32_t fCeilPow2;
            uint32_t fExpectedApprox;
            bool fIsPow2;
        } tests[] = {
            { 0, 0, 1, 16, false },
            { 1, 1, 1, 16, true },
            { 2, 2, 2, 16, true },
            { 3, 2, 4, 16, false },
            { 4, 4, 4, 16, true },
            { 5, 4, 8, 16, false },

            { 7, 4, 8, 16, false },
            { 8, 8, 8, 16, true },
            { 9, 8, 16, 16, false },

            { 15, 8, 16, 16, false },
            { 16, 16, 16, 16, true },
            { 17, 16, 32, 32, false },

            { 31, 16, 32, 32, false },
            { 32, 32, 32, 32, true },
            { 33, 32, 64, 64, false },

            { 1024-1, 512,  1024, 1024, false },
            { 1024,   1024, 1024, 1024, true },
            { 1024+1, 1024, 2048, 1024+512, false },
            { 1024+511, 1024, 2048, 1024+512, false },
            { 1024+512, 1024, 2048, 1024+512, false },
            { 1024+513, 1024, 2048, 2048, false },

            { 2147483648-1, 1073741824, 2147483648, 2147483648, false },
            { 2147483648, 2147483648, 2147483648, 2147483648, true },
        };

        for (int i = 0; i < SK_ARRAY_COUNT(tests); ++i) {
            uint32_t low = GrPrevPow2(tests[i].fVal);
            uint32_t high = GrNextPow2(tests[i].fVal);
            uint32_t approx = TestingOnly_Make_Approx(tests[i].fVal);

            SkDebugf("%d: %u %u (%u) %u (%u) %u (%u)\n", i, tests[i].fVal,
                low, tests[i].fFloorPow2,
                high, tests[i].fCeilPow2,
                approx, tests[i].fExpectedApprox);
            SkASSERT(tests[i].fIsPow2 == SkIsPow2(tests[i].fVal));
            SkASSERT(low == tests[i].fFloorPow2);
            SkASSERT(high == tests[i].fCeilPow2);
            SkASSERT(approx == tests[i].fExpectedApprox);
        }

        SkRandom rand;

        SkRect rect = SkRect::MakeXYWH(10, 10, 200, 200);

        SkPaint p;

        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(35);
        int xOffset = 0, yOffset = 0;
        int direction = 0;

        for (float arc = 134.0f; arc < 136.0f; arc += 0.01f) {
            SkColor color = rand.nextU();
            color |= 0xff000000;
            p.setColor(color);

            canvas->save();
            canvas->translate(SkIntToScalar(xOffset), SkIntToScalar(yOffset));
            canvas->drawArc(rect, 0, arc, false, p);
            canvas->restore();

            switch (direction) {
            case 0:
                xOffset += 10;
                if (xOffset >= 700) {
                    direction = 1;
                }
                break;
            case 1:
                xOffset -= 10;
                yOffset += 10;
                if (xOffset < 50) {
                    direction = 2;
                }
                break;
            case 2:
                xOffset += 10;
                break;
            }
        }

    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
