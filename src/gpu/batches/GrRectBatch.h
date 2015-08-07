/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRectBatch_DEFINED
#define GrRectBatch_DEFINED

#include "GrColor.h"

class GrBatch;
class SkMatrix;
struct SkRect;

/*
 * A factory for returning batches which can draw rectangles.  Right now this only handles non-AA
 * rects
 */
namespace GrRectBatch {

GrBatch* Create(GrColor color,
                const SkMatrix& viewMatrix,
                const SkRect& rect,
                const SkRect* localRect,
                const SkMatrix* localMatrix);

};

#endif
