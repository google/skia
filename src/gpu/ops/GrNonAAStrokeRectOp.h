/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNonAAStrokeRectOp_DEFINED
#define GrNonAAStrokeRectOp_DEFINED

#include <memory>
#include "GrTypes.h"

class GrDrawOp;
class GrPaint;
struct SkRect;
class SkStrokeRec;
class SkMatrix;

namespace GrNonAAStrokeRectOp {

std::unique_ptr<GrDrawOp> Make(GrPaint&& paint,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkStrokeRec&,
                                         bool snapToPixelCenters);
}

#endif
