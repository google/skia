/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"

// Tests clipRRect cases that Graphite should handle with an analytic shader.
DEF_SIMPLE_GM(clip_rrects_analytic, canvas, 360, 325) {
    const SkRect baseRect = {0, 0, 32, 32};

    auto makeRRect = [&](bool tl, bool tr, bool br, bool bl) {
        const float cornerRadius = 8;
        const SkVector radii[4] = {
            {tl ? cornerRadius : 0.f, tl ? cornerRadius : 0.f},
            {tr ? cornerRadius : 0.f, tr ? cornerRadius : 0.f},
            {br ? cornerRadius : 0.f, br ? cornerRadius : 0.f},
            {bl ? cornerRadius : 0.f, bl ? cornerRadius : 0.f}
        };
        return SkRRect::MakeRectRadii(baseRect, radii);
    };

    // Test all combinations of circular corners
    SkRRect rrects[] = {
        makeRRect(false, false, false, false),

        makeRRect(true,  false, false, false),
        makeRRect(false, true,  false, false),
        makeRRect(false, false, true,  false),
        makeRRect(false, false, false, true),

        makeRRect(true,  true,  false, false),
        makeRRect(true,  false, true,  false),
        makeRRect(true,  false, false, true),
        makeRRect(false, true,  true,  false),
        makeRRect(false, true,  false, true),
        makeRRect(false, false, true,  true),

        makeRRect(true,  true,  true,  false),
        makeRRect(true,  true,  false, true),
        makeRRect(true,  false, true,  true),
        makeRRect(false, true,  true,  true),

        makeRRect(true,  true,  true,  true)
    };

    std::pair<SkPoint, SkMatrix> transforms[] = {
        // Identity (top-left grid)
        {{0.f, 0.f},    SkMatrix()},
        // Non-uniform scale (top-right grid)
        {{160.f, 0.f},  SkMatrix::Scale(1.2f, 0.8f)},
        // Orthgonal (bot-left grid)
        {{20.f, 160.f}, SkMatrix::RotateDeg(10.f, {2 * baseRect.width(), 2 * baseRect.height()})},
        // Fully affine (bot-right grid)
        {{180.f, 140.f}, SkMatrix::Skew(0.2f, 0.1f)}
    };

    // Make this a complicated enough shape that Graphite doesn't just apply the clip geometrically.
    SkPathBuilder b;
    b.addRect(baseRect);
    b.addRect(baseRect.makeInset(10, 10));
    b.setFillType(SkPathFillType::kEvenOdd);
    SkPath p = b.detach();

    SkPaint fillPaint;
    fillPaint.setColor(SK_ColorBLACK);
    fillPaint.setAntiAlias(true);

    for (auto [origin, xform] : transforms) {
        canvas->save();
        canvas->translate(origin.fX, origin.fY);

        int x = 0;
        int y = 0;
        for (const SkRRect& clip : rrects) {
            canvas->save();
            canvas->concat(xform);
            canvas->translate(5.f * (x + 1) + baseRect.width() * x,
                              5.f * (y + 1) + baseRect.height() * y);
            canvas->clipRRect(clip, true);
            canvas->drawPath(p, fillPaint);
            canvas->restore();

            x++;
            if (x >= 4) {
                x = 0;
                y++;
            }
        }

        canvas->restore();
    }
}
