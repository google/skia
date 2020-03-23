/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawVerticesOp_DEFINED
#define GrDrawVerticesOp_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkVertices.h"
#include "include/private/GrTypesPriv.h"

class GrColorSpaceXform;
class GrDrawOp;
class GrPaint;
class GrRecordingContext;
class SkRuntimeEffect;

namespace GrDrawVerticesOp {

    /**
     * Draw a SkVertices. The GrPaint param's color is used if the vertices lack per-vertex color.
     * If the vertices lack local coords then the vertex positions are used as local coords. The
     * primitive type drawn is derived from the SkVertices object, unless overridePrimType is
     * specified. If an SkRuntimeEffect is provided, it may expect some number of input varyings,
     * which should match the number of extra per-vertex values in the SkVertices.
     */
    std::unique_ptr<GrDrawOp> Make(GrRecordingContext*,
                                   GrPaint&&,
                                   sk_sp<SkVertices>,
                                   const SkMatrix& viewMatrix,
                                   GrAAType,
                                   sk_sp<GrColorSpaceXform>,
                                   GrPrimitiveType* overridePrimType = nullptr,
                                   const SkRuntimeEffect* = nullptr);

};

#endif
