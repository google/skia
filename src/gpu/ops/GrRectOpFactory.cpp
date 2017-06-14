/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrRectOpFactory.h"

#include "GrAAStrokeRectOp.h"
#include "GrMeshDrawOp.h"
#include "SkStrokeRec.h"

namespace GrRectOpFactory {

std::unique_ptr<GrLegacyMeshDrawOp> MakeAAFillNestedRects(GrColor color,
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
        return GrAAFillRectOp::Make(color, viewMatrix, devOutside, devOutside);
    }

    return GrAAStrokeRectOp::MakeFillBetweenRects(color, viewMatrix, devOutside, devInside);
}
};
