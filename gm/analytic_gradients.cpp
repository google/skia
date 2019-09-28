/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This GM presents a variety of gradients meant to test the correctness of the analytic
 * GrUnrolledBinaryGradientColorizer, which can handle arbitrary gradients with 1 to 8 interpolation
 * intervals. These intervals can be either hardstops or smooth color transitions.
 *
 * It produces an image similar to that of GM_hardstop_gradients, but is arranged as follows:
 *
 *            | Clamp          |
 *            |________________|
 *            | M1  M2  M3  M4 |
 * ___________|________________|
 *      1     |
 *      2     |
 *      3     |
 *      4     |
 *      5     |
 *      6     |
 *      7     |
 *      8     |
 * The M-modes are different ways of interlveaving hardstops with smooth transitions:
 *   - M1 = All smooth transitions
 *   - M2 = All hard stops
 *   - M5 = Alternating smooth then hard
 *   - M6 = Alternating hard then smooth
 *
 * Only clamping is tested since this is focused more on within the interpolation region behavior,
 * compared to overall behavior.
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
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTemplates.h"

// All positions must be divided by the target interval count, which will produce the expected
// normalized position array for that interval number (assuming an appropriate color count is
// provided).
const int M1_POSITIONS[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
const int M2_POSITIONS[] = { 0, 1,1, 2,2, 3,3, 4,4, 5,5, 6,6, 7,7, 8 };
const int M3_POSITIONS[] = { 0, 1, 2,2, 3, 4,4, 5, 6,6, 7, 8 };
const int M4_POSITIONS[] = { 0, 1,1, 2, 3,3, 4, 5,5, 6, 7,7, 8 };

// Color count = index of first occurrence of interval count value in Mx_POSITIONS array.
const int INT1_COLOR_COUNTS[] = { 2, 2, 2, 2 };
const int INT2_COLOR_COUNTS[] = { 3, 4, 3, 4 };
const int INT3_COLOR_COUNTS[] = { 4, 6, 5, 5 };
const int INT4_COLOR_COUNTS[] = { 5, 8, 6, 7 };
const int INT5_COLOR_COUNTS[] = { 6, 10, 8, 8 };
const int INT6_COLOR_COUNTS[] = { 7, 12, 9, 10 };
const int INT7_COLOR_COUNTS[] = { 8, 14, 11, 11 };
const int INT8_COLOR_COUNTS[] = { 9, 16, 12, 13 };

// Cycle through defined colors for positions 0 through 8.
const SkColor COLORS[] = {
    SK_ColorDKGRAY,
    SK_ColorRED,
    SK_ColorYELLOW,
    SK_ColorGREEN,
    SK_ColorCYAN,
    SK_ColorBLUE,
    SK_ColorMAGENTA,
    SK_ColorBLACK,
    SK_ColorLTGRAY
};

const int* INTERVAL_COLOR_COUNTS[] = {
    INT1_COLOR_COUNTS,
    INT2_COLOR_COUNTS,
    INT3_COLOR_COUNTS,
    INT4_COLOR_COUNTS,
    INT5_COLOR_COUNTS,
    INT6_COLOR_COUNTS,
    INT7_COLOR_COUNTS,
    INT8_COLOR_COUNTS
};
const int COLOR_COUNT = SK_ARRAY_COUNT(COLORS);

const int* M_POSITIONS[] = {
    M1_POSITIONS,
    M2_POSITIONS,
    M3_POSITIONS,
    M4_POSITIONS
};

const int WIDTH  = 500;
const int HEIGHT = 500;

const int NUM_ROWS = 8;
const int NUM_COLS = 4;

const int CELL_WIDTH  = WIDTH  / NUM_COLS;
const int CELL_HEIGHT = HEIGHT / NUM_ROWS;

const int PAD_WIDTH  = 3;
const int PAD_HEIGHT = 3;

const int RECT_WIDTH  = CELL_WIDTH  - (2 * PAD_WIDTH);
const int RECT_HEIGHT = CELL_HEIGHT - (2 * PAD_HEIGHT);

static void shade_rect(SkCanvas* canvas, sk_sp<SkShader> shader, int cellRow, int cellCol) {
    SkPaint paint;
    paint.setShader(shader);

    canvas->save();
    canvas->translate(SkIntToScalar(cellCol * CELL_WIDTH + PAD_WIDTH),
                      SkIntToScalar(cellRow * CELL_HEIGHT + PAD_HEIGHT));

    const SkRect rect = SkRect::MakeWH(SkIntToScalar(RECT_WIDTH), SkIntToScalar(RECT_HEIGHT));
    canvas->drawRect(rect, paint);
    canvas->restore();
}

class AnalyticGradientShaderGM : public skiagm::GM {
public:
    AnalyticGradientShaderGM() {

    }

protected:
    SkString onShortName() override {
        return SkString("analytic_gradients");
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 512);
    }

    void onDraw(SkCanvas* canvas) override {
        const SkPoint points[2] = { SkPoint::Make(0, 0), SkPoint::Make(RECT_WIDTH, 0.0) };

        for (int cellRow = 0; cellRow < NUM_ROWS; cellRow++) {
            // Each interval has 4 different color counts, one per mode
            const int* colorCounts = INTERVAL_COLOR_COUNTS[cellRow]; // Has len = 4

            for (int cellCol = 0; cellCol < NUM_COLS; cellCol++) {
                // create_gradient_points(cellRow, cellCol, points);

                // Get the color count dependent on interval and mode
                int colorCount = colorCounts[cellCol];
                // Get the positions given the mode
                const int* layout = M_POSITIONS[cellCol];

                // Collect positions and colors specific to the interval+mode normalizing the
                // position based on the interval count (== cellRow+1)
                SkAutoSTMalloc<4, SkColor> colors(colorCount);
                SkAutoSTMalloc<4, SkScalar> positions(colorCount);
                int j = 0;
                for (int i = 0; i < colorCount; i++) {
                    positions[i] = SkIntToScalar(layout[i]) / (cellRow + 1);
                    colors[i] = COLORS[j % COLOR_COUNT];
                    j++;
                }

                auto shader = SkGradientShader::MakeLinear(
                                points,
                                colors.get(),
                                positions.get(),
                                colorCount,
                                SkTileMode::kClamp,
                                0,
                                nullptr);

                shade_rect(canvas, shader, cellRow, cellCol);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new AnalyticGradientShaderGM;)
