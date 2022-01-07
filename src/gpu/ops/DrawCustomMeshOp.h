/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DrawCustomMeshOp_DEFINED
#define DrawCustomMeshOp_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkCustomMesh.h"
#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrColorSpaceXform;
class GrPaint;
class GrRecordingContext;
class SkMatrixProvider;

namespace skgpu::v1::DrawCustomMeshOp {
GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 SkCustomMesh,
                 const SkMatrixProvider&,
                 GrAAType,
                 sk_sp<GrColorSpaceXform>);

GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 sk_sp<SkVertices>,
                 const GrPrimitiveType* overridePrimitiveType,
                 const SkMatrixProvider&,
                 GrAAType,
                 sk_sp<GrColorSpaceXform>);
}  // namespace skgpu::v1::DrawCustomMeshOp

#endif  // DrawCustomMeshOp_DEFINED
