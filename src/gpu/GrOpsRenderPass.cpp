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

    if (programInfo.hasDynamicPrimProcTextures()) {
        for (int m = 0; m < meshCount; ++m) {
            auto dynamicPrimProcTextures = programInfo.dynamicPrimProcTextures(m);

            for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
                auto* tex = dynamicPrimProcTextures[s]->peekTexture();
                assertResolved(tex, programInfo.primProc().textureSampler(s).samplerState());
            }
        }
    } else if (programInfo.hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = programInfo.fixedPrimProcTextures();

        for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
            auto* tex = fixedPrimProcTextures[s]->peekTexture();
            assertResolved(tex, programInfo.primProc().textureSampler(s).samplerState());
        }
    }

    GrFragmentProcessor::Iter iter(programInfo.pipeline());
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int s = 0; s < fp->numTextureSamplers(); ++s) {
            const auto& textureSampler = fp->textureSampler(s);
            assertResolved(textureSampler.peekTexture(), textureSampler.samplerState());
        }
    }
}
#endif

bool GrOpsRenderPass::draw(const GrProgramInfo& programInfo,
                           const GrMesh meshes[], int meshCount, const SkRect& bounds) {
    if (!meshCount) {
        return true;
    }

#ifdef SK_DEBUG
    SkASSERT(!programInfo.primProc().hasInstanceAttributes() ||
             this->gpu()->caps()->instanceAttribSupport());
    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(programInfo.primProc().hasVertexAttributes() == meshes[i].hasVertexData());
        SkASSERT(programInfo.primProc().hasInstanceAttributes() == meshes[i].hasInstanceData());
    }

    SkASSERT(!programInfo.pipeline().isScissorEnabled() || programInfo.fixedDynamicState() ||
             (programInfo.dynamicStateArrays() && programInfo.dynamicStateArrays()->fScissorRects));

    SkASSERT(!programInfo.pipeline().isBad());

    if (programInfo.hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = programInfo.fixedPrimProcTextures();
        for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
            SkASSERT(fixedPrimProcTextures[s]->isInstantiated());
        }
    }

    if (programInfo.hasDynamicPrimProcTextures()) {
        for (int m = 0; m < meshCount; ++m) {
            auto dynamicPrimProcTextures = programInfo.dynamicPrimProcTextures(m);
            for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
                SkASSERT(dynamicPrimProcTextures[s]->isInstantiated());
            }
        }

        // Check that, for a given sampler, the properties of the dynamic textures remain
        // the same for all the meshes
        for (int s = 0; s < programInfo.primProc().numTextureSamplers(); ++s) {
            auto dynamicPrimProcTextures = programInfo.dynamicPrimProcTextures(0);

            const GrBackendFormat& format = dynamicPrimProcTextures[s]->backendFormat();
            GrTextureType type = dynamicPrimProcTextures[s]->textureType();
            GrPixelConfig config = dynamicPrimProcTextures[s]->config();

            for (int m = 1; m < meshCount; ++m) {
                dynamicPrimProcTextures = programInfo.dynamicPrimProcTextures(m);

                auto testProxy = dynamicPrimProcTextures[s];
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
    GrProcessor::CustomFeatures processorFeatures = programInfo.requestedFeatures();
    if (GrProcessor::CustomFeatures::kSampleLocations & processorFeatures) {
        // Verify we always have the same sample pattern key, regardless of graphics state.
        SkASSERT(this->gpu()->findOrAssignSamplePatternKey(fRenderTarget)
                         == fRenderTarget->renderTargetPriv().getSamplePatternKey());
    }
#endif
    return true;
}
