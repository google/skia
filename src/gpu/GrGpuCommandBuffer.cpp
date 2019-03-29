/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrGpuCommandBuffer.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrFixedClip.h"
#include "GrGpu.h"
#include "GrMesh.h"
#include "GrPrimitiveProcessor.h"
#include "GrRenderTarget.h"
#include "SkRect.h"

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

    auto resourceProvider = this->gpu()->getContext()->priv().resourceProvider();

    if (pipeline.isBad()) {
        return false;
    }
    if (fixedDynamicState && fixedDynamicState->fPrimitiveProcessorTextures) {
        GrTextureProxy** processorProxies = fixedDynamicState->fPrimitiveProcessorTextures;
        for (int i = 0; i < primProc.numTextureSamplers(); ++i) {
            if (resourceProvider->explicitlyAllocateGPUResources()) {
                SkASSERT(processorProxies[i]->isInstantiated());
            } else if (!processorProxies[i]->instantiate(resourceProvider)) {
                return false;
            }
        }
    }
    if (dynamicStateArrays && dynamicStateArrays->fPrimitiveProcessorTextures) {
        int n = primProc.numTextureSamplers() * meshCount;
        const auto* textures = dynamicStateArrays->fPrimitiveProcessorTextures;
        for (int i = 0; i < n; ++i) {
            if (resourceProvider->explicitlyAllocateGPUResources()) {
                SkASSERT(textures[i]->isInstantiated());
            } else if (!textures[i]->instantiate(resourceProvider)) {
                return false;
            }
        }
#ifdef SK_DEBUG
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
#endif

    }

    if (primProc.numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(primProc, pipeline, fixedDynamicState, dynamicStateArrays, meshes, meshCount,
                 bounds);
    return true;
}
