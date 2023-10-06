/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "src/utils/SkPatchUtils.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

static sk_sp<SkShader> make_shader() {
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorCYAN, SK_ColorGREEN, SK_ColorWHITE, SK_ColorMAGENTA, SK_ColorBLUE,
        SK_ColorYELLOW,
    };
    const SkPoint pts[] = { { 100.f / 4.f, 0.f }, { 3.f * 100.f / 4.f, 100.f } };

    return SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors),
                                        SkTileMode::kMirror);
}

static void draw_control_points(SkCanvas* canvas, const SkPoint cubics[12]) {
    //draw control points
    SkPaint paint;
    SkPoint bottom[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetBottomCubic(cubics, bottom);
    SkPoint top[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetTopCubic(cubics, top);
    SkPoint left[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetLeftCubic(cubics, left);
    SkPoint right[SkPatchUtils::kNumPtsCubic];
    SkPatchUtils::GetRightCubic(cubics, right);

    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(0.5f);
    SkPoint corners[4] = { bottom[0], bottom[3], top[0], top[3] };
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, bottom, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, bottom + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, top, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, left, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 4, right, paint);

    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, top + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, left + 1, paint);
    canvas->drawPoints(SkCanvas::kLines_PointMode, 2, right + 1, paint);

    paint.setStrokeWidth(2);

    paint.setColor(SK_ColorRED);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, corners, paint);

    paint.setColor(SK_ColorBLUE);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, bottom + 1, paint);

    paint.setColor(SK_ColorCYAN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, top + 1, paint);

    paint.setColor(SK_ColorYELLOW);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, left + 1, paint);

    paint.setColor(SK_ColorGREEN);
    canvas->drawPoints(SkCanvas::kPoints_PointMode, 2, right + 1, paint);
}

// The order of the colors and points is clockwise starting at upper-left corner.
const SkPoint gCubics[SkPatchUtils::kNumCtrlPts] = {
    //top points
    {100,100},{150,50},{250,150}, {300,100},
    //right points
    {250, 150},{350,250},
    //bottom points
    {300,300},{250,250},{150,350},{100,300},
    //left points
    {50,250},{150,150}
};

const SkPoint gTexCoords[SkPatchUtils::kNumCorners] = {
    {0.0f, 0.0f}, {100.0f, 0.0f}, {100.0f,100.0f}, {0.0f, 100.0f}
};


static void dopatch(SkCanvas* canvas, const SkColor colors[], sk_sp<SkImage> img,
                    const SkMatrix* localMatrix) {
    SkPaint paint;
    paint.setColor(SK_ColorGREEN);

    const SkBlendMode modes[] = {
        SkBlendMode::kSrc,
        SkBlendMode::kDst,
        SkBlendMode::kColorDodge,
    };

    SkPoint texStorage[4];
    const SkPoint* tex = gTexCoords;

    sk_sp<SkShader> shader;
    if (img) {
        SkScalar w = img->width();
        SkScalar h = img->height();
        shader = img->makeShader(SkSamplingOptions(), localMatrix);
        texStorage[0].set(0, 0);
        texStorage[1].set(w, 0);
        texStorage[2].set(w, h);
        texStorage[3].set(0, h);
        tex = texStorage;
    } else {
        shader = make_shader();
    }

    canvas->save();
    for (int y = 0; y < 3; y++) {
        for (int x = 0; x < 4; x++) {
            canvas->save();
            canvas->translate(x * 350.0f, y * 350.0f);
            switch (x) {
                case 0:
                    canvas->drawPatch(gCubics, nullptr, nullptr, modes[y], paint);
                    break;
                case 1:
                    canvas->drawPatch(gCubics, colors, nullptr, modes[y], paint);
                    break;
                case 2:
                    paint.setShader(shader);
                    canvas->drawPatch(gCubics, nullptr, tex, modes[y], paint);
                    paint.setShader(nullptr);
                    break;
                case 3:
                    paint.setShader(shader);
                    canvas->drawPatch(gCubics, colors, tex, modes[y], paint);
                    paint.setShader(nullptr);
                    break;
                default:
                    break;
            }

            draw_control_points(canvas, gCubics);
            canvas->restore();
        }
    }
    canvas->restore();
}

DEF_SIMPLE_GM(patch_primitive, canvas, 1500, 1100) {
    const SkColor colors[SkPatchUtils::kNumCorners] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorCYAN
    };
    dopatch(canvas, colors, nullptr, nullptr);
}
DEF_SIMPLE_GM(patch_image, canvas, 1500, 1100) {
    const SkColor colors[SkPatchUtils::kNumCorners] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorCYAN
    };
    dopatch(canvas, colors, ToolUtils::GetResourceAsImage("images/mandrill_128.png"), nullptr);
}
DEF_SIMPLE_GM(patch_image_persp, canvas, 1500, 1100) {
    const SkColor colors[SkPatchUtils::kNumCorners] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorCYAN
    };
    SkMatrix localM;
    localM.reset();
    localM[6] = 0.00001f;    // force perspective
    dopatch(canvas, colors, ToolUtils::GetResourceAsImage("images/mandrill_128.png"), &localM);
}
DEF_SIMPLE_GM(patch_alpha, canvas, 1500, 1100) {
    const SkColor colors[SkPatchUtils::kNumCorners] = {
        SK_ColorRED, 0x0000FF00, SK_ColorBLUE, 0x00FF00FF,
    };
    dopatch(canvas, colors, nullptr, nullptr);
}

// These two should look the same (one patch, one simple path)
DEF_SIMPLE_GM(patch_alpha_test, canvas, 550, 250) {
    canvas->translate(-75, -75);

    const SkColor colors[SkPatchUtils::kNumCorners] = {
        0x80FF0000, 0x80FF0000, 0x80FF0000, 0x80FF0000,
    };
    SkPaint paint;
    canvas->drawPatch(gCubics, colors, nullptr, SkBlendMode::kDst, paint);

    canvas->translate(300, 0);

    SkPath path;
    path.moveTo(gCubics[0]);
    path.cubicTo(gCubics[ 1], gCubics[ 2], gCubics[ 3]);
    path.cubicTo(gCubics[ 4], gCubics[ 5], gCubics[ 6]);
    path.cubicTo(gCubics[ 7], gCubics[ 8], gCubics[ 9]);
    path.cubicTo(gCubics[10], gCubics[11], gCubics[ 0]);
    paint.setColor(colors[0]);
    canvas->drawPath(path, paint);
}

