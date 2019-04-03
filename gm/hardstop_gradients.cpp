/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/*
 * This GM presents a variety of different gradients with different
 * tile modes. Each entry in the table is a rectangle with a linear
 * gradient that spans from its left edge to its right edge. The rows
 * in the table represent different color/position configurations,
 * while the columns in the table represent different tile modes. In
 * order to highlight the differences between tile modes, the gradient
 * starts and ends at 30 pixel inset from either side of the rectangle.
 *
 *                              | Clamp         Repeat          Mirror
 * _____________________________|___________________________________________
 * 2-color                      | rect00        rect01          rect02
 * 3-color even                 | rect10        rect11          rect12
 * 3-color texture              | rect20        rect21          rect22
 * 5-color hard stop            | rect30        rect31          rect32
 * 4-color hard stop centered   | rect40        rect41          rect42
 * 3-color hard stop 001        | rect50        rect51          rect52
 * 3-color hard stop 011        | rect60        rect61          rect62
 * 4-color hard stop off-center | rect70        rect71          rect72
 *
 * The first three rows are cases covered by pre-hard-stop code; simple
 * 2-color gradients, 3-color gradients with the middle color centered,
 * and general gradients that are rendered from a texture atlas.
 *
 * The next four rows all deal with hard stop gradients. The fourth row
 * is a generic hard stop gradient, while the three subsequent rows deal
 * with special cases of hard stop gradients; centered hard stop gradients
 * (with t-values 0, 0.5, 0.5, 1), and two edge cases (with t-values
 * 0, 0, 1 and 0, 1, 1). The final row has a single off-center hard stop.
 */

#include "gm.h"

#include "SkGradientShader.h"

const int WIDTH  = 500;
const int HEIGHT = 500;

const int NUM_ROWS = 8;
const int NUM_COLS = 3;

const int CELL_WIDTH  = WIDTH  / NUM_COLS;
const int CELL_HEIGHT = HEIGHT / NUM_ROWS;

const int PAD_WIDTH  = 3;
const int PAD_HEIGHT = 3;

const int RECT_WIDTH  = CELL_WIDTH  - (2 * PAD_WIDTH);
const int RECT_HEIGHT = CELL_HEIGHT - (2 * PAD_HEIGHT);

static void shade_rect(SkCanvas* canvas, sk_sp<SkShader> shader, int cellRow, int cellCol) {
    SkPaint paint;
    paint.setShader(shader);

    SkRect rect = SkRect::MakeXYWH(SkIntToScalar(cellCol * CELL_WIDTH  + PAD_WIDTH),
                                   SkIntToScalar(cellRow * CELL_HEIGHT + PAD_HEIGHT),
                                   SkIntToScalar(RECT_WIDTH),
                                   SkIntToScalar(RECT_HEIGHT));

    canvas->drawRect(rect, paint);
}

static void create_gradient_points(int cellRow, int cellCol, SkPoint points[2]) {
    const int X_OFFSET = 30;

    auto x0 = SkIntToScalar(cellCol     * CELL_WIDTH  + PAD_WIDTH  + X_OFFSET);
    auto x1 = SkIntToScalar((cellCol+1) * CELL_WIDTH  - PAD_WIDTH  - X_OFFSET);
    auto y  = SkIntToScalar(cellRow     * CELL_HEIGHT + PAD_HEIGHT + RECT_HEIGHT/2);

    points[0] = SkPoint::Make(x0, y);
    points[1] = SkPoint::Make(x1, y);
}

class HardstopGradientShaderGM : public skiagm::GM {
public:
    HardstopGradientShaderGM() {

    }

protected:
    SkString onShortName() override {
        return SkString("hardstop_gradients");
    }

    SkISize onISize() override {
        return SkISize::Make(512, 512);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPoint points[2];

        SkColor colors[] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorYELLOW,
            SK_ColorMAGENTA,
        };

        SkScalar row3[] = {0.00f, 0.25f, 1.00f};
        SkScalar row4[] = {0.00f, 0.25f, 0.50f, 0.50f, 1.00f};
        SkScalar row5[] = {0.00f, 0.50f, 0.50f, 1.00f};
        SkScalar row6[] = {0.00f, 0.00f, 1.00f};
        SkScalar row7[] = {0.00f, 1.00f, 1.00f};
        SkScalar row8[] = {0.00f, 0.30f, 0.30f, 1.00f};

        SkScalar* positions[NUM_ROWS] = {
            nullptr,
            nullptr,
            row3,
            row4,
            row5,
            row6,
            row7,
            row8,
        };

        int numGradientColors[NUM_ROWS] = {
            2,
            3,
            3,
            5,
            4,
            3,
            3,
            4,
        };

        SkTileMode tilemodes[NUM_COLS] = {
            SkTileMode::kClamp,
            SkTileMode::kRepeat,
            SkTileMode::kMirror,
        };

        for (int cellRow = 0; cellRow < NUM_ROWS; cellRow++) {
            for (int cellCol = 0; cellCol < NUM_COLS; cellCol++) {
                create_gradient_points(cellRow, cellCol, points);

                auto shader = SkGradientShader::MakeLinear(
                                points,
                                colors,
                                positions[cellRow],
                                numGradientColors[cellRow],
                                tilemodes[cellCol],
                                0,
                                nullptr);

                shade_rect(canvas, shader, cellRow, cellCol);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new HardstopGradientShaderGM;)
