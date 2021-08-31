/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DrawAtlasOp_DEFINED
#define DrawAtlasOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class SkMatrix;

namespace skgpu::v1::DrawAtlasOp {

GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMatrix& viewMatrix,
                 GrAAType,
                 int spriteCount,
                 const SkRSXform* xforms,
                 const SkRect* rects,
                 const SkColor* colors);

}  // namespace skgpu::v1::DrawAtlasOp

#endif // DrawAtlasOp_DEFINED
