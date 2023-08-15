/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This GM creates the same gradients as the Chromium test fillrect_gradient:
 * http://osscs/chromium/chromium/src/+/main:third_party/blink/web_tests/fast/canvas/fillrect_gradient.html
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

const int kCellSize = 50;
const int kNumColumns = 2;
const int kNumRows = 9;
const int kPadSize = 10;

class FillrectGradientGM : public skiagm::GM {
public:
    FillrectGradientGM() {}

protected:
    struct GradientStop {
        float pos;
        SkColor color;
    };

    SkString getName() const override { return SkString("fillrect_gradient"); }

    SkISize getISize() override {
        return SkISize::Make(kNumColumns * (kCellSize + kPadSize),
                             kNumRows * (kCellSize + kPadSize));
    }

    void drawGradient(SkCanvas* canvas, std::initializer_list<GradientStop> stops) {
        std::vector<SkColor> colors;
        std::vector<SkScalar> positions;
        colors.reserve(stops.size());
        positions.reserve(stops.size());

        for (const GradientStop& stop : stops) {
            colors.push_back(stop.color);
            positions.push_back(stop.pos);
        }

        static constexpr SkPoint points[] = {
            SkPoint::Make(kCellSize, 0),
            SkPoint::Make(kCellSize, kCellSize),
        };

        // Draw the gradient linearly.
        sk_sp<SkShader> shader = SkGradientShader::MakeLinear(points,
                                                              colors.data(),
                                                              positions.data(),
                                                              colors.size(),
                                                              SkTileMode::kClamp);
        SkPaint paint;
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, kCellSize, kCellSize), paint);

        canvas->save();
        canvas->translate(kCellSize + kPadSize, 0);

        // Draw the gradient radially.
        shader = SkGradientShader::MakeRadial(SkPoint::Make(kCellSize / 2, kCellSize / 2),
                                              kCellSize / 2,
                                              colors.data(),
                                              positions.data(),
                                              colors.size(),
                                              SkTileMode::kClamp);
        paint.setShader(shader);
        canvas->drawRect(SkRect::MakeXYWH(0, 0, kCellSize, kCellSize), paint);

        canvas->restore();
        canvas->translate(0, kCellSize + kPadSize);
    }

    void onDraw(SkCanvas* canvas) override {
        // Simple gradient: Green to white
        this->drawGradient(canvas, {{0.0f, SK_ColorGREEN}, {1.0f, SK_ColorWHITE}});

        // Multiple sections: Green to white to red
        this->drawGradient(canvas,
                           {{0.0f, SK_ColorGREEN}, {0.5f, SK_ColorWHITE}, {1.0f, SK_ColorRED}});

        // No stops at 0.0 or 1.0: Larger green to white to larger red
        this->drawGradient(canvas,
                           {{0.4f, SK_ColorGREEN}, {0.5f, SK_ColorWHITE}, {0.6f, SK_ColorRED}});

        // Only one stop, at zero: Solid red
        this->drawGradient(canvas, {{0.0f, SK_ColorRED}});

        // Only one stop, at 1.0: Solid red
        this->drawGradient(canvas, {{1.0f, SK_ColorRED}});

        // Only one stop, in the middle: Solid red
        this->drawGradient(canvas, {{0.5f, SK_ColorRED}});

        // Disjoint gradients (multiple stops at the same offset)
        // Blue to white in the top (inner) half, red to yellow in the bottom (outer) half
        this->drawGradient(canvas,
                           {{0.0f, SK_ColorBLUE},
                            {0.5f, SK_ColorWHITE},
                            {0.5f, SK_ColorRED},
                            {1.0f, SK_ColorYELLOW}});

        // Ignored stops: Blue to white, red to yellow (same as previous)
        this->drawGradient(canvas,
                           {{0.0f, SK_ColorBLUE},
                            {0.5f, SK_ColorWHITE},
                            {0.5f, SK_ColorGRAY},
                            {0.5f, SK_ColorCYAN},
                            {0.5f, SK_ColorRED},
                            {1.0f, SK_ColorYELLOW}});

        // Unsorted stops: Blue to white, red to yellow
        // Unlike Chrome, we don't sort the stops, so this renders differently than the prior cell.
        this->drawGradient(canvas,
                           {{0.5f, SK_ColorWHITE},
                            {0.5f, SK_ColorGRAY},
                            {1.0f, SK_ColorYELLOW},
                            {0.5f, SK_ColorCYAN},
                            {0.5f, SK_ColorRED},
                            {0.0f, SK_ColorBLUE}});
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM(return new FillrectGradientGM;)
