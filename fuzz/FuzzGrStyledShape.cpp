/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "fuzz/Fuzz.h"
#include "fuzz/FuzzCanvasHelpers.h"
#include "fuzz/FuzzCommon.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "src/gpu/ganesh/geometry/GrStyledShape.h"

DEF_FUZZ(GrStyledShape, fuzz) {
    SkPathBuilder pb;
    FuzzNicePath(fuzz, &pb, 10);

    SkRect rect;
    fuzz->next(&rect);
    if (!rect.isFinite()) {
        return;
    }

    SkRRect rrect;
    fuzz->next(&rrect);
    if (!rrect.isValid()) {
        return;
    }

    SkPaint paint;
    FuzzPaint(fuzz, &paint, 3);

    GrStyledShape shape1(pb.detach(), paint);
    GrStyledShape shape2(rect, paint);
    GrStyledShape shape3(rrect, paint);

    SkPathBuilder pb2;
    FuzzNicePath(fuzz, &pb2, 10);
    SkPath path = pb.detach();
    GrStyledShape shape4(path);
    GrStyledShape shape5(rect);
    GrStyledShape shape6(rrect);

    GrStyledShape shape7(SkPath::Rect(rect));
    GrStyledShape shape8(SkPath::RRect(rrect));
    GrStyledShape shape9(SkPath::Oval(rect));
    GrStyledShape shape10 = GrStyledShape::MakeFilled(shape4);

    auto exercise_shape = [&](const GrStyledShape& shape) {
        shape.style();
        shape.simplified();
        GrStyle::Apply apply;
        fuzz->nextEnum(&apply, GrStyle::Apply::kPathEffectAndStrokeRec);
        SkScalar scale;
        fuzz->nextRange(&scale, 0.001f, 20.0f);
        shape.applyStyle(apply, scale);
        shape.isRect();
        SkRRect r;
        bool inverted;
        shape.asRRect(&r, &inverted);
        SkPoint pts[2];
        shape.asLine(pts, &inverted);
        SkRect rects[2];
        shape.asNestedRects(rects);
        SkPath p = shape.asPath();
        SkASSERT_RELEASE(p.isValid());
        shape.isEmpty();
        shape.bounds();
        shape.styledBounds();
        shape.knownToBeConvex();
        shape.knownDirection();
        shape.inverseFilled();
        shape.mayBeInverseFilledAfterStyling();
        shape.knownToBeClosed();
        shape.segmentMask();

    };

    exercise_shape(shape1);
    exercise_shape(shape2);
    exercise_shape(shape3);
    exercise_shape(shape4);
    exercise_shape(shape5);
    exercise_shape(shape6);
    exercise_shape(shape7);
    exercise_shape(shape8);
    exercise_shape(shape9);
    exercise_shape(shape10);
}
