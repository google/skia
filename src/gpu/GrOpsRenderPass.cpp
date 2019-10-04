/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/GrOpsRenderPass.h"

#include "include/core/SkRect.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrFixedClip.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPrimitiveProcessor.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrTexturePriv.h"

void GrOpsRenderPass::clear(const GrFixedClip& clip, const SkPMColor4f& color) {
    SkASSERT(fRenderTarget);
    // A clear at this level will always be a true clear, so make sure clears were not supposed to
    // be redirected to draws instead
    SkASSERT(!this->gpu()->caps()->performColorClearsAsDraws());
    SkASSERT(!clip.scissorEnabled() || !this->gpu()->caps()->performPartialClearsAsDraws());
    this->onClear(clip, color);
}

void GrOpsRenderPass::clearStencilClip(const GrFixedClip& clip, bool insideStencilMask) {
    // As above, make sure the stencil clear wasn't supposed to be a draw rect with stencil settings
    SkASSERT(!this->gpu()->caps()->performStencilClearsAsDraws());
    this->onClearStencilClip(clip, insideStencilMask);
}

#ifdef SK_DEBUG
static void assert_msaa_and_mips_are_resolved(const GrProgramInfo& programInfo, int meshCount) {
    auto assertResolved = [](GrTexture* tex, const GrSamplerState& sampler) {
        SkASSERT(tex);

        // Ensure mipmaps were all resolved ahead of time by the DAG.
        if (GrSamplerState::Filter::kMipMap == sampler.filter() &&
            (tex->width() != 1 || tex->height() != 1)) {
            // There are some cases where we might be given a non-mipmapped texture with a mipmap
            // filter. See skbug.com/7094.
            SkASSERT(tex->texturePriv().mipMapped() != GrMipMapped::kYes ||
                     !tex->texturePriv().mipMapsAreDirty());
        }
    };

    if (programInfo.dynamicStateArrays() &&
        programInfo.dynamicStateArrays()->fPrimitiveProcessorTextures) {
        for (int m = 0, i = 0; m < meshCount; ++m) {
            for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s, ++i) {
                auto* tex = programInfo.dynamicStateArrays()->fPrimitiveProcessorTextures[i]->peekTexture();
                assertResolved(tex, programInfo.primProc().textureSampler(s).samplerState());
            }
        }
    } else {
        for (int i = 0; i < programInfo.primProc().numTextureSamplers(); ++i) {
            auto* tex = programInfo.fixedDynamicState()->fPrimitiveProcessorTextures[i]->peekTexture();
            assertResolved(tex, programInfo.primProc().textureSampler(i).samplerState());
        }
    }

    GrFragmentProcessor::Iter iter(programInfo.pipeline());
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int i = 0; i < fp->numTextureSamplers(); ++i) {
            const auto& textureSampler = fp->textureSampler(i);
            assertResolved(textureSampler.peekTexture(), textureSampler.samplerState());
        }
    }
}
#endif

bool GrOpsRenderPass::draw(const GrProgramInfo& programInfo,
                           const GrMesh meshes[], int meshCount, const SkRect& bounds) {
#ifdef SK_DEBUG
    SkASSERT(!programInfo.primProc().hasInstanceAttributes() ||
             this->gpu()->caps()->instanceAttribSupport());
    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(!GrPrimTypeRequiresGeometryShaderSupport(meshes[i].primitiveType()) ||
                 this->gpu()->caps()->shaderCaps()->geometryShaderSupport());
        SkASSERT(programInfo.primProc().hasVertexAttributes() == meshes[i].hasVertexData());
        SkASSERT(programInfo.primProc().hasInstanceAttributes() == meshes[i].hasInstanceData());
    }

    SkASSERT(!programInfo.pipeline().isScissorEnabled() || programInfo.fixedDynamicState() ||
             (programInfo.dynamicStateArrays() && programInfo.dynamicStateArrays()->fScissorRects));

    SkASSERT(!programInfo.pipeline().isBad());

    if (programInfo.fixedDynamicState() &&
        programInfo.fixedDynamicState()->fPrimitiveProcessorTextures) {
        GrTextureProxy** processorProxies = programInfo.fixedDynamicState()->fPrimitiveProcessorTextures;
        for (int i = 0; i < programInfo.primProc().numTextureSamplers(); ++i) {
            SkASSERT(processorProxies[i]->isInstantiated());
        }
    }
    if (programInfo.dynamicStateArrays() &&
        programInfo.dynamicStateArrays()->fPrimitiveProcessorTextures) {
        int n = programInfo.primProc().numTextureSamplers() * meshCount;
        const auto* textures = programInfo.dynamicStateArrays()->fPrimitiveProcessorTextures;
        for (int i = 0; i < n; ++i) {
            SkASSERT(textures[i]->isInstantiated());
        }
        SkASSERT(meshCount >= 1);
        const GrTextureProxy* const* primProcProxies =
            programInfo.dynamicStateArrays()->fPrimitiveProcessorTextures;
        for (int i = 0; i < programInfo.primProc().numTextureSamplers(); ++i) {
            const GrBackendFormat& format = primProcProxies[i]->backendFormat();
            GrTextureType type = primProcProxies[i]->textureType();
            GrPixelConfig config = primProcProxies[i]->config();
            for (int j = 1; j < meshCount; ++j) {
                const GrTextureProxy* testProxy =
                        primProcProxies[j*programInfo.primProc().numTextureSamplers() + i];
                SkASSERT(testProxy->backendFormat() == format);
                SkASSERT(testProxy->textureType() == type);
                SkASSERT(testProxy->config() == config);
            }
        }
    }

    assert_msaa_and_mips_are_resolved(programInfo, meshCount);
#endif

    if (programInfo.primProc().numVertexAttributes() > this->gpu()->caps()->maxVertexAttributes()) {
        this->gpu()->stats()->incNumFailedDraws();
        return false;
    }
    this->onDraw(programInfo, meshes, meshCount, bounds);
#ifdef SK_DEBUG
    GrProcessor::CustomFeatures processorFeatures = programInfo.primProc().requestedFeatures();
    for (int i = 0; i < programInfo.pipeline().numFragmentProcessors(); ++i) {
        processorFeatures |= programInfo.pipeline().getFragmentProcessor(i).requestedFeatures();
    }
    processorFeatures |= programInfo.pipeline().getXferProcessor().requestedFeatures();
    if (GrProcessor::CustomFeatures::kSampleLocations & processorFeatures) {
        // Verify we always have the same sample pattern key, regardless of graphics state.
        SkASSERT(this->gpu()->findOrAssignSamplePatternKey(fRenderTarget)
                         == fRenderTarget->renderTargetPriv().getSamplePatternKey());
    }
#endif
    return true;
}
