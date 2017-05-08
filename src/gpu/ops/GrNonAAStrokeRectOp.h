/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAAStrokeRectOp_DEFINED
#define GrNonAAStrokeRectOp_DEFINED

#include "GrColor.h"
#include "SkRefCnt.h"

class GrLegacyMeshDrawOp;
struct SkRect;
class SkStrokeRec;
class SkMatrix;

namespace GrNonAAStrokeRectOp {

std::unique_ptr<GrLegacyMeshDrawOp> Make(GrColor color,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkStrokeRec&,
                                         bool snapToPixelCenters);
}

#endif
