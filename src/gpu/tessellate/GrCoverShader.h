/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCoverShader_DEFINED
#define GrCoverShader_DEFINED

#include "src/gpu/GrGeometryProcessor.h"

// Draws a path's bounding box, with a subpixel outset to avoid possible T-junctions with extreme
// edges of the path. This class is used for the "cover" pass of stencil-then-cover path rendering.
// NOTE: The emitted geometry may not be axis-aligned, depending on the view matrix.
class GrCoverShader : public GrGeometryProcessor {
public:
    GrCoverShader(const SkMatrix& viewMatrix, const SkRect& pathBounds, const SkPMColor4f& color)
            : GrGeometryProcessor(kGrCoverShader_ClassID)
            , fViewMatrix(viewMatrix)
            , fPathBounds(pathBounds)
            , fColor(color) {}

    const char* name() const override { return "GrCoverShader"; }
    void getGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const final {}
    GrGLSLPrimitiveProcessor* createGLSLInstance(const GrShaderCaps&) const final;

private:
    const SkMatrix fViewMatrix;
    const SkRect fPathBounds;
    const SkPMColor4f fColor;

    class Impl;
};

#endif
