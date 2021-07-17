/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

namespace {
static SkSize computeSize(const SkBitmap& bm, const SkMatrix& mat) {
    SkRect bounds = SkRect::MakeWH(SkIntToScalar(bm.width()),
                                   SkIntToScalar(bm.height()));
    mat.mapRect(&bounds);
    return SkSize::Make(bounds.width(), bounds.height());
}

static void draw_cell(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx,
                      const SkSamplingOptions& sampling) {
    SkAutoCanvasRestore acr(canvas, true);

    canvas->translate(dx, 0);
    canvas->concat(mat);
    canvas->drawImage(bm.asImage(), 0, 0, sampling);
}

static void draw_row(SkCanvas* canvas, const SkBitmap& bm, const SkMatrix& mat, SkScalar dx) {
    draw_cell(canvas, bm, mat, 0 * dx, SkSamplingOptions());
    draw_cell(canvas, bm, mat, 1 * dx, SkSamplingOptions(SkFilterMode::kLinear));
    draw_cell(canvas, bm, mat, 2 * dx, SkSamplingOptions(SkFilterMode::kLinear,
                                                         SkMipmapMode::kLinear));
    draw_cell(canvas, bm, mat, 3 * dx, SkSamplingOptions(SkCubicResampler::Mitchell()));
}

class FilterIndiaBoxGM : public skiagm::GM {
    SkBitmap    fBM;
    SkMatrix    fMatrix[2];

    void onOnceBeforeDraw() override {
        constexpr char kResource[] = "images/box.gif";
        if (!GetResourceAsBitmap(kResource, &fBM)) {
            fBM.allocN32Pixels(1, 1);
            fBM.eraseARGB(255, 255, 0 , 0); // red == bad
        }
        fBM.setImmutable();

        SkScalar cx = SkScalarHalf(fBM.width());
        SkScalar cy = SkScalarHalf(fBM.height());

        float vertScale = 30.0f/55.0f;
        float horizScale = 150.0f/200.0f;

        fMatrix[0].setScale(horizScale, vertScale);
        fMatrix[1].setRotate(30, cx, cy); fMatrix[1].postScale(horizScale, vertScale);
    }

    SkString onShortName() override { return SkString("filterindiabox"); }

    SkISize onISize() override { return {680, 130}; }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(10, 10);
        for (size_t i = 0; i < SK_ARRAY_COUNT(fMatrix); ++i) {
            SkSize size = computeSize(fBM, fMatrix[i]);
            size.fWidth += 20;
            size.fHeight += 20;

            draw_row(canvas, fBM, fMatrix[i], size.fWidth);
            canvas->translate(0, size.fHeight);
        }
    }
};
}  // namespace

DEF_GM( return new FilterIndiaBoxGM(); )
