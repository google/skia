/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAStrokeRectBatch_DEFINED
#define GrAAStrokeRectBatch_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrDrawOp;
class GrResourceProvider;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

namespace GrAAStrokeRectOp {

sk_sp<GrDrawOp> MakeFillBetweenRects(GrColor color,
                                     const SkMatrix& viewMatrix,
                                     const SkRect& devOutside,
                                     const SkRect& devInside);

sk_sp<GrDrawOp> Make(GrColor color,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect,
                     const SkStrokeRec& stroke);
}

#endif
