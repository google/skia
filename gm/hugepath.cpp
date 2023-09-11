/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTiledImageUtils.h"

DEF_SIMPLE_GM(path_huge_crbug_800804, canvas, 50, 600) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    // exercise various special-cases (e.g. hairlines or not)
    const float widths[] = { 0.9f, 1.0f, 1.1f };

    SkPath path;
    for (float w : widths) {
        paint.setStrokeWidth(w);

        path.reset();
        path.moveTo(-1000,12345678901234567890.f);
        path.lineTo(10.5f,200);
        canvas->drawPath(path, paint);

        path.reset();
        path.moveTo(30.5f,400);
        path.lineTo(1000,-9.8765432109876543210e+19f);
        canvas->drawPath(path, paint);

        canvas->translate(3, 0);
    }
}

// Test that we can draw into a huge surface ( > 64K ) and still retain paths and antialiasing.
static void draw_huge_path(SkCanvas* canvas, int w, int h, bool manual) {
    SkAutoCanvasRestore acr(canvas, true);

    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h));
    auto can = surf->getCanvas();

    SkPaint paint;
    SkPath path;
    path.addRoundRect(SkRect::MakeXYWH(4, 4, w - 8, h - 8), 12, 12);

    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(4, 4, 64, 64));
    can->drawPath(path, paint);
    if (manual) {
        SkTiledImageUtils::DrawImage(canvas, surf->makeImageSnapshot(), 64 - w, 0);
    } else {
        canvas->drawImage(surf->makeImageSnapshot(), 64 - w, 0);
    }
    canvas->restore();

    canvas->translate(80, 0);
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(4, 4, 64, 64));
    can->clear(0);
    paint.setAntiAlias(true);
    can->drawPath(path, paint);
    if (manual) {
        SkTiledImageUtils::DrawImage(canvas, surf->makeImageSnapshot(), 64 - w, 0);
    } else {
        canvas->drawImage(surf->makeImageSnapshot(), 64 - w, 0);
    }
    canvas->restore();
};

DEF_SIMPLE_GM(path_huge_aa, canvas, 200, 200) {
    draw_huge_path(canvas, 100, 60, /* manual= */ false);
    canvas->translate(0, 80);
    draw_huge_path(canvas, 100 * 1024, 60, /* manual= */ false);
}

DEF_SIMPLE_GM(path_huge_aa_manual, canvas, 200, 200) {
    draw_huge_path(canvas, 100, 60, /* manual= */ true);
    canvas->translate(0, 80);
    draw_huge_path(canvas, 100 * 1024, 60, /* manual= */ true);
}
