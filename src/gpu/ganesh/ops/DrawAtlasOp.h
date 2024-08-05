/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DrawAtlasOp_DEFINED
#define DrawAtlasOp_DEFINED

#include "include/core/SkColor.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class SkMatrix;
enum class GrAAType : unsigned int;
struct SkRSXform;
struct SkRect;

namespace skgpu::ganesh::DrawAtlasOp {

GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMatrix& viewMatrix,
                 GrAAType,
                 int spriteCount,
                 const SkRSXform* xforms,
                 const SkRect* rects,
                 const SkColor* colors);

}  // namespace skgpu::ganesh::DrawAtlasOp

#endif // DrawAtlasOp_DEFINED
