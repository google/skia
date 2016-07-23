/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPatchGrid.h"

static void draw_control_points(SkCanvas* canvas, const SkPoint cubics[12]) {
    //draw control points
    SkPaint paint;
    SkPoint bottom[4];
    SkPatchUtils::getBottomCubic(cubics, bottom);
    SkPoint top[4];
    SkPatchUtils::getTopCubic(cubics, top);
    SkPoint left[4];
    SkPatchUtils::getLeftCubic(cubics, left);
    SkPoint right[4];
    SkPatchUtils::getRightCubic(cubics, right);

    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(0.5);
    SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom+1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, paint);

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top+1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left+1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right+1, paint);

    paint.setStrokeWidth(2);

    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, corners, paint);

    paint.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, bottom+1, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, top+1, paint);

    paint.setColor(SK_ColorYELLOW);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, left+1, paint);

    paint.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, right+1, paint);
}

namespace skiagm {
/**
 * This GM draws a grid of patches, it only uses colors so it could be considered a mesh gradient.
 */
class SkPatchGridGM : public GM {

public:
    SkPatchGridGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("patch_grid");
    }

    SkISize onISize() override {
        return SkISize::Make(800, 800);
    }

    void onDraw(SkCanvas* canvas) override {

        SkPaint paint;

        SkPoint vertices[4][5] = {
            {{50,50}, {150,50}, {250,50},{350,50},{450,50}},
            {{50,150}, {120,120}, {250,150},{350,150},{450,150}},
            {{50,250}, {150,250}, {250,250},{350,250},{450,250}},
            {{100,300}, {150,350}, {250,350},{350,350},{450,350}}
        };

        SkColor cornerColors[4][5] = {
            {SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE},
            {SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED},
            {SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE},
            {SK_ColorRED, SK_ColorBLUE, SK_ColorRED, SK_ColorBLUE, SK_ColorRED},
        };

        SkPoint hrzCtrl[4][8] = {
            {{75,30},{125,45},{175,70},{225,20},{275,50},{325,50},{375,5},{425,90}},
            {{75,150},{125,150},{175,150},{225,150},{275,150},{325,150},{375,150},{425,150}},
            {{75,250},{125,250},{175,250},{225,250},{275,200},{325,150},{375,250},{425,250}},
            {{75,350},{125,350},{175,350},{225,350},{275,350},{325,350},{375,350},{425,350}}
        };

        SkPoint vrtCtrl[6][5] = {
            {{50,75},{150,75},{250,75},{350,75},{450,75}},
            {{50,125},{150,125},{250,125},{350,125},{450,125}},
            {{50,175},{150,175},{220,225},{350,175},{470,225}},
            {{50,225},{150,225},{220,175},{350,225},{470,155}},
            {{50,275},{150,275},{250,275},{350,275},{400,305}},
            {{50,325},{150,325},{250,325},{350,325},{450,325}}
        };

        static const int kRows = 3;
        static const int kCols = 4;

        canvas->scale(3, 3);
        SkPatchGrid grid(kRows, kCols, SkPatchGrid::kColors_VertexType, nullptr);
        for (int i = 0; i < kRows; i++) {
            for (int j = 0; j < kCols; j++) {
                SkPoint points[12];

                //set corners
                points[SkPatchUtils::kTopP0_CubicCtrlPts] = vertices[i][j];
                points[SkPatchUtils::kTopP3_CubicCtrlPts] = vertices[i][j + 1];
                points[SkPatchUtils::kBottomP0_CubicCtrlPts] = vertices[i + 1][j];
                points[SkPatchUtils::kBottomP3_CubicCtrlPts] = vertices[i + 1][j + 1];

                points[SkPatchUtils::kTopP1_CubicCtrlPts] = hrzCtrl[i][j * 2];
                points[SkPatchUtils::kTopP2_CubicCtrlPts] = hrzCtrl[i][j * 2 + 1];
                points[SkPatchUtils::kBottomP1_CubicCtrlPts] = hrzCtrl[i + 1][j * 2];
                points[SkPatchUtils::kBottomP2_CubicCtrlPts] = hrzCtrl[i + 1][j * 2 + 1];

                points[SkPatchUtils::kLeftP1_CubicCtrlPts] = vrtCtrl[i * 2][j];
                points[SkPatchUtils::kLeftP2_CubicCtrlPts] = vrtCtrl[i * 2 + 1][j];
                points[SkPatchUtils::kRightP1_CubicCtrlPts] = vrtCtrl[i * 2][j + 1];
                points[SkPatchUtils::kRightP2_CubicCtrlPts] = vrtCtrl[i * 2 + 1][j + 1];

                SkColor colors[4];
                colors[0] = cornerColors[i][j];
                colors[1] = cornerColors[i][j + 1];
                colors[3] = cornerColors[i + 1][j];
                colors[2] = cornerColors[i + 1][j + 1];

                grid.setPatch(j, i, points, colors, nullptr);
            }
        }

        grid.draw(canvas, paint);
        SkISize dims = grid.getDimensions();
        for (int y = 0; y < dims.height(); y++) {
            for (int x = 0; x < dims.width(); x++) {
                SkPoint cubics[12];
                grid.getPatch(x, y, cubics, nullptr, nullptr);
                draw_control_points(canvas, cubics);
            }
        }
    }

private:
    typedef GM INHERITED;
};

DEF_GM(return new SkPatchGridGM;)
}
