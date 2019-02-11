/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDrawVerticesOp_DEFINED
#define GrDrawVerticesOp_DEFINED

#include "GrTypesPriv.h"
#include "SkRefCnt.h"
#include "SkVertices.h"

class GrColorSpaceXform;
class GrContext;
class GrDrawOp;
class GrPaint;

namespace GrDrawVerticesOp {

    /**
     * Draw a SkVertices. The GrPaint param's color is used if the vertices lack per-vertex color.
     * If the vertices lack local coords then the vertex positions are used as local coords. The
     * primitive type drawn is derived from the SkVertices object, unless overridePrimType is
     * specified.
     */
    std::unique_ptr<GrDrawOp> Make(GrContext*,
                                   GrPaint&&,
                                   sk_sp<SkVertices>,
                                   const SkVertices::Bone bones[],
                                   int boneCount,
                                   const SkMatrix& viewMatrix,
                                   GrAAType,
                                   sk_sp<GrColorSpaceXform>,
                                   GrPrimitiveType* overridePrimType = nullptr);
};

#endif
