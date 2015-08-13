/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrBWFillRectBatch_DEFINED
#define GrBWFillRectBatch_DEFINED

#include "GrColor.h"

class GrBatch;
class SkMatrix;
struct SkRect;

namespace GrBWFillRectBatch {
GrBatch* Create(GrColor color,
                const SkMatrix& viewMatrix,
                const SkRect& rect,
                const SkRect* localRect,
                const SkMatrix* localMatrix);
};

#endif
