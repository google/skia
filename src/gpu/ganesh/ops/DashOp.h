/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_ganesh_DashOp_DEFINED
#define skgpu_ganesh_DashOp_DEFINED

#include "src/gpu/ganesh/ops/GrOp.h"

class GrPaint;
class GrRecordingContext;
class GrStyle;
class SkMatrix;
struct GrUserStencilSettings;
struct SkPoint;

namespace skgpu::ganesh::DashOp {

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

}  // namespace skgpu::ganesh::DashOp

#endif // skgpu_ganesh_DashOp_DEFINED
