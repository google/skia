/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAAStrokeRectOp_DEFINED
#define GrAAStrokeRectOp_DEFINED

#include <memory>
#include "GrTypes.h"

class GrDrawOp;
class GrPaint;
class SkMatrix;
struct SkRect;
class SkStrokeRec;

namespace GrAAStrokeRectOp {

std::unique_ptr<GrDrawOp> MakeFillBetweenRects(GrPaint&&,
                                                         const SkMatrix& viewMatrix,
                                                         const SkRect& devOutside,
                                                         const SkRect& devInside);

std::unique_ptr<GrDrawOp> Make(GrPaint&&,
                                         const SkMatrix& viewMatrix,
                                         const SkRect& rect,
                                         const SkStrokeRec& stroke);
}

#endif
