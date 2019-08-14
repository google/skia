/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrGpuCommandBuffer.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"

void GrGpuRTCommandBuffer::clear(const GrFixedClip& clip, const SkPMColor4f& color) {
    SkASSERT(fRenderTarget);
    // A clear at this level will always be a true clear, so make sure clears were not supposed to
    // be redirected to draws instead
    SkASSERT(!this->gpu()->caps()->performColorClearsAsDraws());
    SkASSERT(!clip.scissorEnabled() || !this->gpu()->caps()->performPartialClearsAsDraws());
    this->onClear(clip, color);
}

void GrGpuRTCommandBuffer::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    // As above, make sure the stencil clear wasn't supposed to be a draw rect with stencil settings
    SkASSERT(!this->gpu()->caps()->performStencilClearsAsDraws());
    this->onClearStencilClip(clip, insideStencilMask);
}

bool GrGpuRTCommandBuffer::draw(const GrPrimitiveProcessor& primProc, const GrPipeline& pipeline,
                                const GrPipeline::FixedDynamicState* fixedDynamicState,
                                const GrPipeline::DynamicStateArrays* dynamicStateArrays,
                                const GrMesh meshes[], int meshCount, const SkRect& bounds) {
#ifdef SK_DEBUG
    SkASSERT(!primProc.hasInstanceAttributes() || this->gpu()->caps()->instanceAttribSupport());
    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(!GrPrimTypeRequiresGeometryShaderSupport(meshes[i].primitiveType()) ||
                 this->gpu()->caps()->shaderCaps()->geometryShaderSupport());
        SkASSERT(primProc.hasVertexAttributes() == meshes[i].hasVertexData());
        SkASSERT(primProc.hasInstanceAttributes() == meshes[i].hasInstanceData());
    }
#endif
    SkASSERT(!pipeline.isScissorEnabled() || fixedDynamicState ||
             (dynamicStateArrays && dynamicStateArrays->fScissorRects));

    SkASSERT(!pipeline.isBad());

#ifdef SK_DEBUG
    if (fixedDynamicState && fixedDynamicState->fPrimitiveProcessorTextures) {
        GrTextureProxy** processorProxies = fixedDynamicState->fPrimitiveProcessorTextures;
        for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
            SkASSERT(processorProxies[i]->isInstantiated());
        }
    }
    if (dynamicStateArrays && dynamicStateArrays->fPrimitiveProcessorTextures) {
        int n = primProc.numTextureSamplers() * meshCount;
        const auto* textures = dynamicStateArrays->fPrimitiveProcessorTextures;
        for (int i = 0; i < n; ++i) {
            SkASSERT(textures[i]->isInstantiated());
        }
        SkASSERT(meshCount >= 1);
        const GrTextureProxy* const* primProcProxies =
                dynamicStateArrays->fPrimitiveProcessorTextures;
        for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
            const GrBackendFormat& format = primProcProxies[i]->backendFormat();
            GrTextureType type = primProcProxies[i]->textureType();
            GrPixelConfig config = primProcProxies[i]->config();
            for (int j = 1; j < meshCount; ++j) {
                const GrTextureProxy* testProxy =
                        primProcProxies[j*primProc.numTextureSamplers() + i];
                SkASSERT(testProxy->backendFormat() == format);
                SkASSERT(testProxy->textureType() == type);
                SkASSERT(testProxy->config() == config);
            }
        }
    }
#endif

    if (primProc.numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(primProc, pipeline, fixedDynamicState, dynamicStateArrays, meshes, meshCount,
                 bounds);
#ifdef SK_DEBUG
    GrProcessor::CustomFeatures processorFeatures = primProc.requestedFeatures();
    for (int i = 0; i < pipeline.numFragmentProcessors(); ++i) {
        processorFeatures |= pipeline.getFragmentProcessor(i).requestedFeatures();
    }
    processorFeatures |= pipeline.getXferProcessor().requestedFeatures();
    if (GrProcessor::CustomFeatures::kSampleLocations & processorFeatures) {
        // Verify we always have the same sample pattern key, regardless of graphics state.
        SkASSERT(this->gpu()->findOrAssignSamplePatternKey(fRenderTarget)
                         == fRenderTarget->renderTargetPriv().getSamplePatternKey());
    }
#endif
    return true;
}
