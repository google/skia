/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef RegionOp_DEFINED
#define RegionOp_DEFINED

#include "src/gpu/ganesh/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class SkMatrix;
class SkRegion;
enum class GrAAType : unsigned int;
struct GrUserStencilSettings;

namespace skgpu::ganesh::RegionOp {

/** GrAAType must be kNone or kMSAA. */
GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMatrix& viewMatrix,
                 const SkRegion&,
                 GrAAType,
                 const GrUserStencilSettings* stencilSettings = nullptr);

}  // namespace skgpu::ganesh::RegionOp

#endif // RegionOp_DEFINED
