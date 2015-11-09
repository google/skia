/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAFillRectBatch_DEFINED
#define GrAAFillRectBatch_DEFINED

#include "GrColor.h"

class GrBatch;
class GrDrawBatch;
class SkMatrix;
struct SkRect;

namespace GrAAFillRectBatch {
GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect& devRect);

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkMatrix& localMatrix,
                    const SkRect& rect);

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkMatrix& localMatrix,
                    const SkRect& rect,
                    const SkRect& devRect);

GrDrawBatch* CreateWithLocalRect(GrColor color,
                                 const SkMatrix& viewMatrix,
                                 const SkRect& rect,
                                 const SkRect& localRect);

void Append(GrBatch*,
            GrColor,
            const SkMatrix& viewMatrix,
            const SkRect& rect,
            const SkRect& devRect);

void Append(GrBatch*,
            GrColor,
            const SkMatrix& viewMatrix,
            const SkMatrix& localMatrix,
            const SkRect& rect,
            const SkRect& devRect);
};

#endif
