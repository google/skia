/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillRRectOp_DEFINED
#define FillRRectOp_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrCaps;
class GrPaint;
class GrRecordingContext;
class SkMatrix;
struct SkRect;

namespace skgpu::v1::FillRRectOp {

    GrOp::Owner Make(GrRecordingContext*,
                     SkArenaAlloc*,
                     GrPaint&&,
                     const SkMatrix& viewMatrix,
                     const SkRRect&,
                     const SkRect& localRect,
                     GrAA);

    GrOp::Owner Make(GrRecordingContext*,
                     SkArenaAlloc*,
                     GrPaint&&,
                     const SkMatrix& viewMatrix,
                     const SkRRect&,
                     const SkMatrix& localMatrix,
                     GrAA);

}  // namespace skgpu::v1::FillRRectOp

#endif // FillRRectOp_DEFINED
