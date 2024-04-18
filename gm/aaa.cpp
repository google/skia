/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkScalar.h"
#include "src/base/SkFloatBits.h"
#include "src/core/SkPathPriv.h"

#define W   800
#define H   800

DEF_SIMPLE_GM(analytic_antialias_convex, canvas, W, H) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);

    canvas->clear(0xFFFFFFFF);

    canvas->save();

    SkScalar y = 0;

    canvas->translate(0, y);
    canvas->rotate(1);
    canvas->drawRect({ 20, 20, 200, 200 }, p);
    canvas->restore();

    y += 200;

    canvas->save();
    canvas->translate(0, y);
    canvas->rotate(1);
    canvas->drawRect({ 20, 20, 20.2f, 200 }, p);
    canvas->drawRect({ 20, 200, 200, 200.1f }, p);
    canvas->drawCircle(100, 100, 30, p);
    canvas->restore();

    // The following path is empty but it'll reveal bug chrome:662914
    SkPathBuilder pb;
    pb.moveTo(SkBits2Float(0x429b9d5c), SkBits2Float(0x4367a041));  // 77.8073f, 231.626f
    // 77.8075f, 231.626f, 77.8074f, 231.625f, 77.8073f, 231.625f
    pb.cubicTo(SkBits2Float(0x429b9d71), SkBits2Float(0x4367a022),
            SkBits2Float(0x429b9d64), SkBits2Float(0x4367a009),
            SkBits2Float(0x429b9d50), SkBits2Float(0x43679ff2));
    pb.lineTo(SkBits2Float(0x429b9d5c), SkBits2Float(0x4367a041));  // 77.8073f, 231.626f
    pb.close();
    canvas->drawPath(pb.detach(), p);

    // The following path reveals a subtle SkAnalyticQuadraticEdge::updateQuadratic bug:
    // we should not use any snapped y for the intermediate values whose error may accumulate;
    // snapping should only be allowed once before updateLine.
    pb.moveTo(SkBits2Float(0x434ba71e), SkBits2Float(0x438a06d0));  // 203.653f, 276.053f
    pb.lineTo(SkBits2Float(0x43492a74), SkBits2Float(0x4396d70d));  // 201.166f, 301.68f
    // 200.921f, 304.207f, 196.939f, 303.82f, 0.707107f
    pb.conicTo(SkBits2Float(0x4348ebaf), SkBits2Float(0x43981a75),
            SkBits2Float(0x4344f079), SkBits2Float(0x4397e900), SkBits2Float(0x3f3504f3));
    pb.close();
    SkPath path = pb.detach();
    // Manually setting convexity is required. Otherwise, this path will be considered concave.
    SkPathPriv::SetConvexity(path, SkPathConvexity::kConvex);
    canvas->drawPath(path, p);

    // skbug.com/7573
    y += 200;
    canvas->save();
    canvas->translate(0, y);
    p.setAntiAlias(true);
    pb.moveTo(1.98009784f, 9.0162744f);
    pb.lineTo(47.843992f, 10.1922744f);
    pb.lineTo(47.804008f, 11.7597256f);
    pb.lineTo(1.93990216f, 10.5837256f);
    canvas->drawPath(pb.detach(), p);
    canvas->restore();

    // skbug.com/7813
    // t8888 splits the 800-high canvas into 3 pieces; the boundary is close to 266 and 534
    pb.moveTo(700, 266);
    pb.lineTo(710, 266);
    pb.lineTo(710, 534);
    pb.lineTo(700, 534);
    canvas->drawPath(pb.detach(), p);
}

DEF_SIMPLE_GM(analytic_antialias_general, canvas, W, H) {
    SkPaint p;
    p.setColor(SK_ColorRED);
    p.setAntiAlias(true);

    canvas->clear(0xFFFFFFFF);

    canvas->save();
    canvas->rotate(1);
    const SkScalar R = 115.2f, C = 128.0f;
    SkPathBuilder builder;
    builder.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
        SkScalar a = 2.6927937f * i;
        builder.lineTo(C + R * SkScalarCos(a), C + R * SkScalarSin(a));
    }
    SkPath path = builder.detach();
    canvas->drawPath(path, p);
    canvas->restore();

    canvas->save();
    canvas->translate(200, 0);
    canvas->rotate(1);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(5);
    canvas->drawPath(path, p);
    canvas->restore();


    // The following two paths test if we correctly cumulates the alpha on the middle pixel
    // column where the left rect and the right rect abut.
    p.setStyle(SkPaint::kFill_Style);
    canvas->translate(0, 300);
    canvas->drawPath(SkPathBuilder().addRect({20, 20, 100.4999f, 100})
                                    .addRect({100.5001f, 20, 200, 100})
                                    .detach(), p);

    canvas->translate(300, 0);
    canvas->drawPath(SkPathBuilder().addRect({20, 20, 100.1f, 100})
                                    .addRect({100.9f, 20, 200, 100})
                                    .detach(), p);
}

DEF_SIMPLE_GM(analytic_antialias_inverse, canvas, W, H) {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);

        canvas->save();

        SkPath path = SkPath::Circle(100, 100, 30);
        path.setFillType(SkPathFillType::kInverseWinding);
        canvas->drawPath(path, p);
        canvas->restore();
}
