/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAAFillRectBatch_DEFINED
#define GrNonAAFillRectBatch_DEFINED

#include "GrColor.h"

class GrDrawBatch;
class SkMatrix;
struct SkRect;

namespace GrNonAAFillRectBatch {

GrDrawBatch* Create(GrColor color,
                    const SkMatrix& viewMatrix,
                    const SkRect& rect,
                    const SkRect* localRect,
                    const SkMatrix* localMatrix);

GrDrawBatch* CreateWithPerspective(GrColor color,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkRect* localRect,
                                   const SkMatrix* localMatrix);

bool Append(GrColor color,
            const SkMatrix& viewMatrix,
            const SkRect& rect,
            const SkRect* localRect,
            const SkMatrix* localMatrix);

};

#endif
