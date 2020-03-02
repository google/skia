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
    GrPrimitiveType primitiveType() const { return fPrimitiveType; }
    int tessellationPatchVertexCount() const { return fTessellationPatchVertexCount; }

    void issueDraw(GrOpFlushState* state, const GrPipeline* pipeline,
                   const GrPipeline::FixedDynamicState* fixedDynamicState,
                   sk_sp<const GrBuffer> vertexBuffer, int vertexCount, int baseVertex,
                   const SkRect& bounds) {
        GrMesh mesh;
        mesh.setNonIndexedNonInstanced(vertexCount);
        mesh.setVertexData(std::move(vertexBuffer), baseVertex);
        this->issueDraw(state, pipeline, fixedDynamicState, mesh, bounds);
    }

    void issueDraw(GrOpFlushState* state, const GrPipeline* pipeline,
                   const GrPipeline::FixedDynamicState* fixedDynamicState, const GrMesh& mesh,
                   const SkRect& bounds) {
        GrProgramInfo programInfo(state->proxy()->numSamples(), state->proxy()->numStencilSamples(),
                                  state->proxy()->backendFormat(), state->view()->origin(),
                                  pipeline, this, fixedDynamicState, nullptr, 0,
                                  fPrimitiveType, fTessellationPatchVertexCount);
        state->opsRenderPass()->bindPipeline(programInfo, bounds);
        state->opsRenderPass()->drawMeshes(programInfo, &mesh, 1);
    }

private:
    const SkMatrix fViewMatrix;
    const GrPrimitiveType fPrimitiveType;
    const int fTessellationPatchVertexCount;

    class Impl;
};

#endif
