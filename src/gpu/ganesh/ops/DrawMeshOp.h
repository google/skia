/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DrawMeshOp_DEFINED
#define DrawMeshOp_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/ops/GrOp.h"

class GrColorSpaceXform;
class GrPaint;
class GrRecordingContext;
class SkMatrix;
class SkMesh;

namespace skgpu::ganesh::DrawMeshOp {
GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 const SkMesh&,
                 skia_private::TArray<std::unique_ptr<GrFragmentProcessor>> children,
                 const SkMatrix&,
                 GrAAType,
                 sk_sp<GrColorSpaceXform>);

GrOp::Owner Make(GrRecordingContext*,
                 GrPaint&&,
                 sk_sp<SkVertices>,
                 const GrPrimitiveType* overridePrimitiveType,
                 const SkMatrix&,
                 GrAAType,
                 sk_sp<GrColorSpaceXform>);
}  // namespace skgpu::ganesh::DrawMeshOp

#endif  // DrawMeshOp_DEFINED
