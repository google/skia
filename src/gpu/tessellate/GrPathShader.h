/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPathShader_DEFINED
#define GrPathShader_DEFINED

#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrOpFlushState.h"
#include "src/gpu/GrOpsRenderPass.h"
#include "src/gpu/GrProgramInfo.h"

 // This is a common base class for shaders in the GPU tessellator.
class GrPathShader : public GrGeometryProcessor {
public:
    GrPathShader(ClassID classID, const SkMatrix& viewMatrix, GrPrimitiveType primitiveType,
                 int tessellationPatchVertexCount)
            : GrGeometryProcessor(classID)
            , fViewMatrix(viewMatrix)
            , fPrimitiveType(primitiveType)
            , fTessellationPatchVertexCount(tessellationPatchVertexCount) {
        if (fTessellationPatchVertexCount) {
            this->setWillUseTessellationShaders();
        }
    }

    const SkMatrix& viewMatrix() const { return fViewMatrix; }

    // This subclass is used to simplify the argument list for constructing GrProgramInfo from a
    // GrPathShader.
    class ProgramInfo : public GrProgramInfo {
    public:
        ProgramInfo(const GrSurfaceProxyView* view, const GrPipeline* pipeline,
                    const GrPathShader* shader)
                : ProgramInfo(view->asRenderTargetProxy(), view->origin(), pipeline, shader) {
        }
        ProgramInfo(const GrRenderTargetProxy* proxy, GrSurfaceOrigin origin,
                    const GrPipeline* pipeline, const GrPathShader* shader)
                : GrProgramInfo(proxy->numSamples(), proxy->numStencilSamples(),
                                proxy->backendFormat(), origin, pipeline, shader,
                                shader->fPrimitiveType, shader->fTessellationPatchVertexCount) {
        }
    };

public:
    // Wang's formula for cubics (1985) gives us the number of evenly spaced (in the
    // parametric sense) line segments that are guaranteed to be within a distance of
    // "MAX_LINEARIZATION_ERROR" from the actual curve.
    constexpr static char kWangsFormulaCubicFn[] = R"(
            float wangs_formula_cubic(vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
                float k = (3.0 * 2.0) / (8.0 * MAX_LINEARIZATION_ERROR);
                float f = sqrt(k * length(max(abs(p2 - p1*2.0 + p0),
                                              abs(p3 - p2*2.0 + p1))));
                return max(1.0, ceil(f));
            })";

private:
    const SkMatrix fViewMatrix;
    const GrPrimitiveType fPrimitiveType;
    const int fTessellationPatchVertexCount;

    class Impl;
};

#endif
