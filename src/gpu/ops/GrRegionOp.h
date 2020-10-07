/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRegionOp_DEFINED
#define GrRegionOp_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrDrawOp;
class GrRecordingContext;
class SkMatrix;
class SkRegion;
class GrPaint;
struct GrUserStencilSettings;

namespace GrRegionOp {
/** GrAAType must be kNone or kMSAA. */
GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMatrix& viewMatrix,
                 const SkRegion&,
                 GrAAType,
                 const GrUserStencilSettings* stencilSettings = nullptr);
}  // namespace GrRegionOp

#endif
