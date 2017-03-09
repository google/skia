/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAStrokeRectOp_DEFINED
#define GrAAStrokeRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrMeshDrawOp;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

namespace GrAAStrokeRectOp {

std::unique_ptr<GrMeshDrawOp> MakeFillBetweenRects(GrColor color,
                                                   const SkMatrix& viewMatrix,
                                                   const SkRect& devOutside,
                                                   const SkRect& devInside);

std::unique_ptr<GrMeshDrawOp> Make(GrColor color,
                                   const SkMatrix& viewMatrix,
                                   const SkRect& rect,
                                   const SkStrokeRec& stroke);
}

#endif
