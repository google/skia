/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawAtlasOp_DEFINED
#define GrDrawAtlasOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrDrawOp;
class GrPaint;
class GrRecordingContext;
class SkMatrix;

namespace GrDrawAtlasOp {
GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMatrix& viewMatrix,
                 GrAAType,
                 int spriteCount,
                 const SkRSXform* xforms,
                 const SkRect* rects,
                 const SkColor* colors);
}  // namespace GrDrawAtlasOp

#endif
