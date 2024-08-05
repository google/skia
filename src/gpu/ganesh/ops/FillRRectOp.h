/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FillRRectOp_DEFINED
#define FillRRectOp_DEFINED

#include "src/gpu/ganesh/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class SkArenaAlloc;
class SkMatrix;
class SkRRect;
enum class GrAA : bool;
struct SkRect;

namespace skgpu::ganesh::FillRRectOp {

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

}  // namespace skgpu::ganesh::FillRRectOp

#endif // FillRRectOp_DEFINED
