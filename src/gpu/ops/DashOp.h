/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DashOp_DEFINED
#define DashOp_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class GrStyle;
struct GrUserStencilSettings;

namespace skgpu::v1::DashOp {

enum class AAMode {
    kNone,
    kCoverage,
    kCoverageWithMSAA,
};

GrOp::Owner MakeDashLineOp(GrRecordingContext*,
                           GrPaint&&,
                           const SkMatrix& viewMatrix,
                           const SkPoint pts[2],
                           AAMode,
                           const GrStyle& style,
                           const GrUserStencilSettings*);
bool CanDrawDashLine(const SkPoint pts[2], const GrStyle& style, const SkMatrix& viewMatrix);

}  // namespace skgpu::v1::DashOp

#endif // DashOp_DEFINED
