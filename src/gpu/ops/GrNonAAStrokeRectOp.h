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

class GrDrawOp;
struct SkRect;
class SkStrokeRec;
class SkMatrix;

namespace GrNonAAStrokeRectOp {

sk_sp<GrDrawOp> Make(GrColor color,
                     const SkMatrix& viewMatrix,
                     const SkRect& rect,
                     const SkStrokeRec&,
                     bool snapToPixelCenters);
}

#endif
