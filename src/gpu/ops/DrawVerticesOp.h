/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DrawVerticesOp_DEFINED
#define DrawVerticesOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/ops/GrOp.h"

class GrColorSpaceXform;
class GrPaint;
class GrRecordingContext;
class SkMatrixProvider;
class SkRuntimeEffect;
class SkVertices;

namespace skgpu::v1::DrawVerticesOp {

    /**
     * Draw a SkVertices. The GrPaint param's color is used if the vertices lack per-vertex color.
     * If the vertices lack local coords then the vertex positions are used as local coords. The
     * primitive type drawn is derived from the SkVertices object, unless overridePrimType is
     * specified. If an SkRuntimeEffect is provided, it may expect some number of input varyings,
     * which should match the number of extra per-vertex values in the SkVertices.
     */
    GrOp::Owner Make(GrRecordingContext*,
                     GrPaint&&,
                     sk_sp<SkVertices>,
                     const SkMatrixProvider&,
                     GrAAType,
                     sk_sp<GrColorSpaceXform>,
                     GrPrimitiveType* overridePrimType,
                     const SkRuntimeEffect*);

}  // namespace skgpu::v1::DrawVerticesOp

#endif // DrawVerticesOp_DEFINED
