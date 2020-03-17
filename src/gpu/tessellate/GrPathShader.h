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

private:
    const SkMatrix fViewMatrix;
    const GrPrimitiveType fPrimitiveType;
    const int fTessellationPatchVertexCount;

    class Impl;
};

#endif
