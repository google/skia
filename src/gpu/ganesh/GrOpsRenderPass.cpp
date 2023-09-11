/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/GrOpsRenderPass.h"

#include "include/core/SkRect.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrCpuBuffer.h"
#include "src/gpu/ganesh/GrDrawIndirectCommand.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrProgramInfo.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrSimpleMesh.h"
#include "src/gpu/ganesh/GrTexture.h"

void GrOpsRenderPass::begin() {
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
#ifdef SK_DEBUG
    fScissorStatus = DynamicStateStatus::kDisabled;
    fTextureBindingStatus = DynamicStateStatus::kDisabled;
    fHasIndexBuffer = false;
    fInstanceBufferStatus = DynamicStateStatus::kDisabled;
    fVertexBufferStatus = DynamicStateStatus::kDisabled;
#endif
    this->onBegin();
}

void GrOpsRenderPass::end() {
    this->onEnd();
    this->resetActiveBuffers();
}

void GrOpsRenderPass::clear(const GrScissorState& scissor, std::array<float, 4> color) {
    SkASSERT(fRenderTarget);
    // A clear at this level will always be a true clear, so make sure clears were not supposed to
    // be redirected to draws instead
    SkASSERT(!this->gpu()->caps()->performColorClearsAsDraws());
    SkASSERT(!scissor.enabled() || !this->gpu()->caps()->performPartialClearsAsDraws());
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    this->onClear(scissor, color);
}

void GrOpsRenderPass::clearStencilClip(const GrScissorState& scissor, bool insideStencilMask) {
    // As above, make sure the stencil clear wasn't supposed to be a draw rect with stencil settings
    SkASSERT(!this->gpu()->caps()->performStencilClearsAsDraws());
    SkASSERT(!scissor.enabled() || !this->gpu()->caps()->performPartialClearsAsDraws());
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    this->onClearStencilClip(scissor, insideStencilMask);
}

void GrOpsRenderPass::executeDrawable(std::unique_ptr<SkDrawable::GpuDrawHandler> drawable) {
    fDrawPipelineStatus = DrawPipelineStatus::kNotConfigured;
    this->onExecuteDrawable(std::move(drawable));
}

void GrOpsRenderPass::bindPipeline(const GrProgramInfo& programInfo, const SkRect& drawBounds) {
#ifdef SK_DEBUG
    // Both the 'programInfo' and this renderPass have an origin. Since they come from the same
    // place (i.e., the target renderTargetProxy) they had best agree.
    SkASSERT(programInfo.origin() == fOrigin);
    if (programInfo.geomProc().hasInstanceAttributes()) {
         SkASSERT(this->gpu()->caps()->drawInstancedSupport());
    }
    if (programInfo.pipeline().usesConservativeRaster()) {
        SkASSERT(this->gpu()->caps()->conservativeRasterSupport());
    }
    if (programInfo.pipeline().isWireframe()) {
         SkASSERT(this->gpu()->caps()->wireframeSupport());
    }
    if (this->gpu()->caps()->twoSidedStencilRefsAndMasksMustMatch() &&
        programInfo.isStencilEnabled()) {
        const GrUserStencilSettings* stencil = programInfo.userStencilSettings();
        if (stencil->isTwoSided(programInfo.pipeline().hasStencilClip())) {
            SkASSERT(stencil->fCCWFace.fRef == stencil->fCWFace.fRef);
            SkASSERT(stencil->fCCWFace.fTestMask == stencil->fCWFace.fTestMask);
            SkASSERT(stencil->fCCWFace.fWriteMask == stencil->fCWFace.fWriteMask);
        }
    }
    programInfo.checkAllInstantiated();
    programInfo.checkMSAAAndMIPSAreResolved();
#endif

    this->resetActiveBuffers();

    if (programInfo.geomProc().numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        fDrawPipelineStatus = DrawPipelineStatus::kFailedToBind;
        return;
    }

    if (!this->onBindPipeline(programInfo, drawBounds)) {
        fDrawPipelineStatus = DrawPipelineStatus::kFailedToBind;
        return;
    }

#ifdef SK_DEBUG
    fScissorStatus = (programInfo.pipeline().isScissorTestEnabled()) ?
            DynamicStateStatus::kUninitialized : DynamicStateStatus::kDisabled;
    bool hasTextures = (programInfo.geomProc().numTextureSamplers() > 0);
    if (!hasTextures) {
        programInfo.pipeline().visitProxies(
                [&hasTextures](GrSurfaceProxy*, skgpu::Mipmapped) { hasTextures = true; });
    }
    fTextureBindingStatus = (hasTextures) ?
            DynamicStateStatus::kUninitialized : DynamicStateStatus::kDisabled;
    fHasIndexBuffer = false;
    fInstanceBufferStatus = (programInfo.geomProc().hasInstanceAttributes()) ?
            DynamicStateStatus::kUninitialized : DynamicStateStatus::kDisabled;
    fVertexBufferStatus = (programInfo.geomProc().hasVertexAttributes()) ?
            DynamicStateStatus::kUninitialized : DynamicStateStatus::kDisabled;
#endif

    fDrawPipelineStatus = DrawPipelineStatus::kOk;
    fXferBarrierType = programInfo.pipeline().xferBarrierType(*this->gpu()->caps());
}

void GrOpsRenderPass::setScissorRect(const SkIRect& scissor) {
    if (DrawPipelineStatus::kOk != fDrawPipelineStatus) {
        SkASSERT(DrawPipelineStatus::kNotConfigured != fDrawPipelineStatus);
        return;
    }
    SkASSERT(DynamicStateStatus::kDisabled != fScissorStatus);
    this->onSetScissorRect(scissor);
    SkDEBUGCODE(fScissorStatus = DynamicStateStatus::kConfigured);
}

void GrOpsRenderPass::bindTextures(const GrGeometryProcessor& geomProc,
                                   const GrSurfaceProxy* const geomProcTextures[],
                                   const GrPipeline& pipeline) {
#ifdef SK_DEBUG
    SkASSERT((geomProc.numTextureSamplers() > 0) == SkToBool(geomProcTextures));
    for (int i = 0; i < geomProc.numTextureSamplers(); ++i) {
        const auto& sampler = geomProc.textureSampler(i);
        const GrSurfaceProxy* proxy = geomProcTextures[i];
        SkASSERT(proxy);
        SkASSERT(proxy->backendFormat() == sampler.backendFormat());
        SkASSERT(proxy->backendFormat().textureType() == sampler.backendFormat().textureType());

        const GrTexture* tex = proxy->peekTexture();
        SkASSERT(tex);
        if (sampler.samplerState().mipmapped() == skgpu::Mipmapped::kYes &&
            (tex->width() != 1 || tex->height() != 1)) {
            // There are some cases where we might be given a non-mipmapped texture with a mipmap
            // filter. See skbug.com/7094.
            SkASSERT(tex->mipmapped() != skgpu::Mipmapped::kYes || !tex->mipmapsAreDirty());
        }
    }
#endif

    if (DrawPipelineStatus::kOk != fDrawPipelineStatus) {
        SkASSERT(DrawPipelineStatus::kNotConfigured != fDrawPipelineStatus);
        return;
    }

    // Don't assert on fTextureBindingStatus. onBindTextures() just turns into a no-op when there
    // aren't any textures, and it's hard to tell from the GrPipeline whether there are any. For
    // many clients it is easier to just always call this method.
    if (!this->onBindTextures(geomProc, geomProcTextures, pipeline)) {
        fDrawPipelineStatus = DrawPipelineStatus::kFailedToBind;
        return;
    }

    SkDEBUGCODE(fTextureBindingStatus = DynamicStateStatus::kConfigured);
}

void GrOpsRenderPass::bindBuffers(sk_sp<const GrBuffer> indexBuffer,
                                  sk_sp<const GrBuffer> instanceBuffer,
                                  sk_sp<const GrBuffer> vertexBuffer,
                                  GrPrimitiveRestart primRestart) {
    if (DrawPipelineStatus::kOk != fDrawPipelineStatus) {
        SkASSERT(DrawPipelineStatus::kNotConfigured != fDrawPipelineStatus);
        return;
    }

#ifdef SK_DEBUG
    if (indexBuffer) {
        fHasIndexBuffer = true;
    }

    SkASSERT((DynamicStateStatus::kDisabled == fInstanceBufferStatus) != SkToBool(instanceBuffer));
    if (instanceBuffer) {
        fInstanceBufferStatus = DynamicStateStatus::kConfigured;
    }

    SkASSERT((DynamicStateStatus::kDisabled == fVertexBufferStatus) != SkToBool(vertexBuffer));
    if (vertexBuffer) {
        fVertexBufferStatus = DynamicStateStatus::kConfigured;
    }

    if (GrPrimitiveRestart::kYes == primRestart) {
        SkASSERT(this->gpu()->caps()->usePrimitiveRestart());
    }
#endif

    this->onBindBuffers(std::move(indexBuffer), std::move(instanceBuffer), std::move(vertexBuffer),
                        primRestart);
}

bool GrOpsRenderPass::prepareToDraw() {
    if (DrawPipelineStatus::kOk != fDrawPipelineStatus) {
        SkASSERT(DrawPipelineStatus::kNotConfigured != fDrawPipelineStatus);
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    SkASSERT(DynamicStateStatus::kUninitialized != fScissorStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fTextureBindingStatus);

    if (kNone_GrXferBarrierType != fXferBarrierType) {
        this->gpu()->xferBarrier(fRenderTarget, fXferBarrierType);
    }
    return true;
}

void GrOpsRenderPass::draw(int vertexCount, int baseVertex) {
    if (!this->prepareToDraw()) {
        return;
    }
    SkASSERT(!fHasIndexBuffer);
    SkASSERT(DynamicStateStatus::kConfigured != fInstanceBufferStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fVertexBufferStatus);
    this->onDraw(vertexCount, baseVertex);
}

void GrOpsRenderPass::drawIndexed(int indexCount, int baseIndex, uint16_t minIndexValue,
                                  uint16_t maxIndexValue, int baseVertex) {
    if (!this->prepareToDraw()) {
        return;
    }
    SkASSERT(fHasIndexBuffer);
    SkASSERT(DynamicStateStatus::kConfigured != fInstanceBufferStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fVertexBufferStatus);
    this->onDrawIndexed(indexCount, baseIndex, minIndexValue, maxIndexValue, baseVertex);
}

void GrOpsRenderPass::drawInstanced(int instanceCount, int baseInstance, int vertexCount,
                                    int baseVertex) {
    SkASSERT(this->gpu()->caps()->drawInstancedSupport());
    if (!this->prepareToDraw()) {
        return;
    }
    SkASSERT(!fHasIndexBuffer);
    SkASSERT(DynamicStateStatus::kUninitialized != fInstanceBufferStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fVertexBufferStatus);
    this->onDrawInstanced(instanceCount, baseInstance, vertexCount, baseVertex);
}

void GrOpsRenderPass::drawIndexedInstanced(int indexCount, int baseIndex, int instanceCount,
                                           int baseInstance, int baseVertex) {
    SkASSERT(this->gpu()->caps()->drawInstancedSupport());
    if (!this->prepareToDraw()) {
        return;
    }
    SkASSERT(fHasIndexBuffer);
    SkASSERT(DynamicStateStatus::kUninitialized != fInstanceBufferStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fVertexBufferStatus);
    this->onDrawIndexedInstanced(indexCount, baseIndex, instanceCount, baseInstance, baseVertex);
}

void GrOpsRenderPass::drawIndirect(const GrBuffer* drawIndirectBuffer, size_t bufferOffset,
                                   int drawCount) {
    SkASSERT(this->gpu()->caps()->drawInstancedSupport());
    SkASSERT(drawIndirectBuffer->isCpuBuffer() ||
             !static_cast<const GrGpuBuffer*>(drawIndirectBuffer)->isMapped());
    if (!this->prepareToDraw()) {
        return;
    }
    SkASSERT(!fHasIndexBuffer);
    SkASSERT(DynamicStateStatus::kUninitialized != fInstanceBufferStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fVertexBufferStatus);
    if (!this->gpu()->caps()->nativeDrawIndirectSupport()) {
        // Polyfill indirect draws with looping instanced calls.
        SkASSERT(drawIndirectBuffer->isCpuBuffer());
        auto* cpuIndirectBuffer = static_cast<const GrCpuBuffer*>(drawIndirectBuffer);
        auto* cmds = reinterpret_cast<const GrDrawIndirectCommand*>(
                cpuIndirectBuffer->data() + bufferOffset);
        for (int i = 0; i < drawCount; ++i) {
            auto [vertexCount, instanceCount, baseVertex, baseInstance] = cmds[i];
            this->onDrawInstanced(instanceCount, baseInstance, vertexCount, baseVertex);
        }
        return;
    }
    this->onDrawIndirect(drawIndirectBuffer, bufferOffset, drawCount);
}

void GrOpsRenderPass::drawIndexedIndirect(const GrBuffer* drawIndirectBuffer, size_t bufferOffset,
                                          int drawCount) {
    SkASSERT(this->gpu()->caps()->drawInstancedSupport());
    SkASSERT(drawIndirectBuffer->isCpuBuffer() ||
             !static_cast<const GrGpuBuffer*>(drawIndirectBuffer)->isMapped());
    if (!this->prepareToDraw()) {
        return;
    }
    SkASSERT(fHasIndexBuffer);
    SkASSERT(DynamicStateStatus::kUninitialized != fInstanceBufferStatus);
    SkASSERT(DynamicStateStatus::kUninitialized != fVertexBufferStatus);
    if (!this->gpu()->caps()->nativeDrawIndirectSupport() ||
        this->gpu()->caps()->nativeDrawIndexedIndirectIsBroken()) {
        // Polyfill indexedIndirect draws with looping indexedInstanced calls.
        SkASSERT(drawIndirectBuffer->isCpuBuffer());
        auto* cpuIndirectBuffer = static_cast<const GrCpuBuffer*>(drawIndirectBuffer);
        auto* cmds = reinterpret_cast<const GrDrawIndexedIndirectCommand*>(
                cpuIndirectBuffer->data() + bufferOffset);
        for (int i = 0; i < drawCount; ++i) {
            auto [indexCount, instanceCount, baseIndex, baseVertex, baseInstance] = cmds[i];
            this->onDrawIndexedInstanced(indexCount, baseIndex, instanceCount, baseInstance,
                                         baseVertex);
        }
        return;
    }
    this->onDrawIndexedIndirect(drawIndirectBuffer, bufferOffset, drawCount);
}

void GrOpsRenderPass::drawIndexPattern(int patternIndexCount, int patternRepeatCount,
                                       int maxPatternRepetitionsInIndexBuffer,
                                       int patternVertexCount, int baseVertex) {
    int baseRepetition = 0;
    while (baseRepetition < patternRepeatCount) {
        int repeatCount = std::min(patternRepeatCount - baseRepetition,
                                   maxPatternRepetitionsInIndexBuffer);
        int drawIndexCount = repeatCount * patternIndexCount;
        // A patterned index buffer must contain indices in the range [0..vertexCount].
        int minIndexValue = 0;
        int maxIndexValue = patternVertexCount * repeatCount - 1;
        this->drawIndexed(drawIndexCount, 0, minIndexValue, maxIndexValue,
                          patternVertexCount * baseRepetition + baseVertex);
        baseRepetition += repeatCount;
    }
}
