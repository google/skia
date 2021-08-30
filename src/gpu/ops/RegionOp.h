/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef RegionOp_DEFINED
#define RegionOp_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
struct GrUserStencilSettings;
class SkMatrix;
class SkRegion;

namespace skgpu::v1::RegionOp {

/** GrAAType must be kNone or kMSAA. */
GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMatrix& viewMatrix,
                 const SkRegion&,
                 GrAAType,
                 const GrUserStencilSettings* stencilSettings = nullptr);

} // namespace skgpu::v1::RegionOp

#endif // RegionOp_DEFINED
