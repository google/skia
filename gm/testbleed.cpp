/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkRSXform.h"
#include "include/core/SkVertices.h"
#include "tools/ToolUtils.h"

constexpr int MARGIN        = 2;
constexpr int CHECKER_SIZE  = 4;
constexpr int CHECKER_COUNT = 4;
constexpr int N             = MARGIN + CHECKER_SIZE * CHECKER_COUNT + MARGIN;

// Create a checker image, with a red border (to detect bleed)
static std::pair<sk_sp<SkImage>, SkRect> make_image() {
    auto surf = SkSurface::MakeRasterN32Premul(N, N);
    SkRect src = {MARGIN, MARGIN, N-MARGIN, N-MARGIN};

    SkCanvas* canvas = surf->getCanvas();
    canvas->drawColor(SK_ColorRED);
    canvas->clipRect(src);
    canvas->translate(2, 2);
    ToolUtils::draw_checkerboard(canvas, SK_ColorBLACK, SK_ColorWHITE, CHECKER_SIZE);
    return std::make_pair(surf->makeImageSnapshot(), src);
}

static sk_sp<SkVertices> make_vertices(const SkRect& r) {
    SkPoint pos[4];
    r.toQuad(pos);
    return SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode, 4, pos, pos, nullptr);
}

static void draw_grid(SkCanvas* canvas, const SkImage* img, const SkRect& src) {
    SkAutoCanvasRestore acr(canvas, true);

    canvas->scale(2.0f/SK_ScalarPI, 2.0f/SK_ScalarPI);

    for (auto fq : {kNone_SkFilterQuality, kLow_SkFilterQuality, kHigh_SkFilterQuality}) {
        canvas->save();

        SkPaint paint;
        paint.setFilterQuality(fq);
        canvas->drawImageRect(img, src, src, &paint, SkCanvas::kStrict_SrcRectConstraint);

        canvas->translate(0, N + 10);

        canvas->drawImageRect(img, src, src, &paint, SkCanvas::kFast_SrcRectConstraint);

        canvas->translate(0, N + 10);

        paint.setShader(img->makeShader());
        canvas->drawVertices(make_vertices(src), paint);

        canvas->translate(0, N + 10);

        paint.setShader(nullptr);
        SkRSXform xform = {1, 0, src.fLeft, src.fTop};
        canvas->drawAtlas(img, &xform, &src, nullptr, 1, SkBlendMode::kSrc, nullptr, &paint);

        canvas->restore();
        canvas->translate(N + 10, 0);
    }
}

/*
 *  Test that we blend when we should (and don't when we shouldn't)
 *
 *  The left-colum is all nearest-neighbor, and the top-row is draw in "strict", so
 *  these are expect to NOT blead. Everthing below and to the right should bleed.
 */
DEF_SIMPLE_GM(test_bleed, canvas, 240, 360) {
    auto [img, src] = make_image();

    canvas->translate(10, 10);
    canvas->drawImage(img, 0, 0, nullptr);

    canvas->translate(0, N + 20);

    SkImageInfo info = SkImageInfo::MakeN32Premul(4*N, 5*N);
    auto surf = canvas->makeSurface(info);
    draw_grid(surf->getCanvas(), img.get(), src);

    canvas->scale(4, 4);
    canvas->drawImage(surf->makeImageSnapshot(), 0, 0, nullptr);
}
