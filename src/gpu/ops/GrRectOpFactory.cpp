/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRectOpFactory.h"
#include "GrAAStrokeRectOp.h"
#include "SkMatrix.h"
#include "SkRect.h"

namespace GrRectOpFactory {

std::unique_ptr<GrDrawOp> MakeAAFillNestedRects(GrPaint&& paint,
                                                          const SkMatrix& viewMatrix,
                                                          const SkRect rects[2]) {
    SkASSERT(viewMatrix.rectStaysRect());
    SkASSERT(!rects[0].isEmpty() && !rects[1].isEmpty());

    SkRect devOutside, devInside;
    viewMatrix.mapRect(&devOutside, rects[0]);
    viewMatrix.mapRect(&devInside, rects[1]);
    if (devInside.isEmpty()) {
        if (devOutside.isEmpty()) {
            return nullptr;
        }
        return GrAAFillRectOp::Make(std::move(paint), viewMatrix, SkMatrix::I(), rects[0],
                                    devOutside);
    }

    return GrAAStrokeRectOp::MakeFillBetweenRects(std::move(paint), viewMatrix, devOutside,
                                                  devInside);
}
};
