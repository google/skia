/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This GM presents different gradients with an increasing number of
 * hardstops, from 1 to 100.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/effects/SkGradientShader.h"

#include <vector>

const int kWidth  = 1000;
const int kHeight = 2000;
const int kNumRows = 100;
const int kCellHeight = kHeight / kNumRows;
const int kPadHeight = 1;
const int kRectHeight = kCellHeight - (2 * kPadHeight);

class HardstopGradientsManyGM : public skiagm::GM {
public:
    HardstopGradientsManyGM() {}

protected:
    SkString getName() const override { return SkString("hardstop_gradients_many"); }

    SkISize getISize() override { return SkISize::Make(kWidth, kHeight); }

    void onDraw(SkCanvas* canvas) override {
        static constexpr SkPoint points[] = {
            SkPoint::Make(0,      kRectHeight / 2),
            SkPoint::Make(kWidth, kRectHeight / 2),
        };

        std::vector<SkColor> colors;
        std::vector<SkScalar> positions;

        for (int row = 1; row <= kNumRows; ++row) {
            // Assemble a gradient containing a blue-to-white blend, repeated N times per row.
            colors.push_back(SK_ColorBLUE);
            colors.push_back(SK_ColorWHITE);

            positions = {0.0f};
            for (int pos = 1; pos < row; ++pos) {
                float place = SkScalar(pos) / SkScalar(row);
                positions.push_back(place);
                positions.push_back(place);
            }
            positions.push_back(1.0f);
            SkASSERT(positions.size() == colors.size());

            // Draw it.
            auto shader = SkGradientShader::MakeLinear(points,
                                                       colors.data(),
                                                       positions.data(),
                                                       colors.size(),
                                                       SkTileMode::kClamp,
                                                       /*flags=*/0,
                                                       /*localMatrix=*/nullptr);
            SkPaint paint;
            paint.setShader(shader);
            canvas->drawRect(SkRect::MakeXYWH(0, kPadHeight, kWidth, kRectHeight), paint);

            canvas->translate(0, kCellHeight);
        }
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new HardstopGradientsManyGM;)
