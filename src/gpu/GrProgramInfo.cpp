/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProgramInfo.h"

#include "src/gpu/GrStencilSettings.h"

GrStencilSettings GrProgramInfo::nonGLStencilSettings() const {
    GrStencilSettings stencil;

    if (this->pipeline().isStencilEnabled()) {
        stencil.reset(*this->pipeline().getUserStencil(),
                      this->pipeline().hasStencilClip(),
                      8);
    }

    return stencil;
}

#ifdef SK_DEBUG
#include "src/gpu/GrTexturePriv.h"

void GrProgramInfo::validate(bool flushTime) const {
    if (flushTime) {
        SkASSERT(fPipeline->allProxiesInstantiated());
    }

    if (this->hasDynamicPrimProcTextures()) {
        SkASSERT(!this->hasFixedPrimProcTextures());
        SkASSERT(fPrimProc->numTextureSamplers());
    } else if (this->hasFixedPrimProcTextures()) {
        SkASSERT(fPrimProc->numTextureSamplers());
    // TODO: We will soon remove dynamic state from GrProgramInfo. But while migrating to the new
    // bind/draw API on GrOpsRenderPass, some code will not set the dynamic state because it calls
    // bindTextures() directly. Once dynamic state (including this validation code) is moved out of
    // GrProgramInfo, we can restore this assert.
    // } else {
    //     SkASSERT(!fPrimProc->numTextureSamplers());
    }


    // TODO: We will soon remove dynamic state from GrProgramInfo. But while migrating to the new
    // bind/draw API on GrOpsRenderPass, some code will not set the dynamic state because it calls
    // setScissorRect() directly. Once dynamic state (including this validation code) is moved out
    // of GrProgramInfo, we can restore this assert.
#if 0
    SkASSERT((fPipeline->isScissorTestEnabled()) ==
             (this->hasFixedScissor() || this->hasDynamicScissors()));
#else
    if (!fPipeline->isScissorTestEnabled()) {
         SkASSERT(!this->hasFixedScissor() && !this->hasDynamicScissors());
    }
#endif

    if (this->hasDynamicPrimProcTextures()) {
        // Check that, for a given sampler, the properties of the dynamic textures remain
        // the same for all the meshes
        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            auto dynamicPrimProcTextures = this->dynamicPrimProcTextures(0);

            const GrBackendFormat& format = dynamicPrimProcTextures[s]->backendFormat();
            GrTextureType type = dynamicPrimProcTextures[s]->backendFormat().textureType();

            for (int m = 1; m < fNumDynamicStateArrays; ++m) {
                dynamicPrimProcTextures = this->dynamicPrimProcTextures(m);

                auto testProxy = dynamicPrimProcTextures[s];
                SkASSERT(testProxy->asTextureProxy());
                SkASSERT(testProxy->backendFormat() == format);
                SkASSERT(testProxy->backendFormat().textureType() == type);
            }
        }
    }
}

void GrProgramInfo::checkAllInstantiated() const {
    if (this->hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = this->fixedPrimProcTextures();
        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            SkASSERT(fixedPrimProcTextures[s]->isInstantiated());
        }
    }

    if (this->hasDynamicPrimProcTextures()) {
        for (int m = 0; m < fNumDynamicStateArrays; ++m) {
            auto dynamicPrimProcTextures = this->dynamicPrimProcTextures(m);
            for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
                SkASSERT(dynamicPrimProcTextures[s]->isInstantiated());
            }
        }
    }
}

void GrProgramInfo::checkMSAAAndMIPSAreResolved() const {
    auto assertResolved = [](GrTexture* tex, GrSamplerState sampler) {
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

    if (this->hasDynamicPrimProcTextures()) {
        for (int m = 0; m < fNumDynamicStateArrays; ++m) {
            auto dynamicPrimProcTextures = this->dynamicPrimProcTextures(m);

            for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
                auto* tex = dynamicPrimProcTextures[s]->peekTexture();
                assertResolved(tex, this->primProc().textureSampler(s).samplerState());
            }
        }
    } else if (this->hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = this->fixedPrimProcTextures();

        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            auto* tex = fixedPrimProcTextures[s]->peekTexture();
            assertResolved(tex, this->primProc().textureSampler(s).samplerState());
        }
    }

    for (auto [sampler, fp] : GrFragmentProcessor::PipelineTextureSamplerRange(this->pipeline())) {
        assertResolved(sampler.peekTexture(), sampler.samplerState());
    }
}

#endif
