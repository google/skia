/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillRRectOp_DEFINED
#define FillRRectOp_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

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

}  // namespace skgpu::v1::FillRRectOp

#endif // FillRRectOp_DEFINED
