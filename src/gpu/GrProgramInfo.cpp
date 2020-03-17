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

    if (this->hasFixedPrimProcTextures()) {
        SkASSERT(fPrimProc->numTextureSamplers());
    }

    if (!fPipeline->isScissorTestEnabled()) {
         SkASSERT(!this->hasFixedScissor());
    }
}

void GrProgramInfo::checkAllInstantiated() const {
    if (this->hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = this->fixedPrimProcTextures();
        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            SkASSERT(fixedPrimProcTextures[s]->isInstantiated());
        }
    }
    for (auto [sampler, fp] : GrFragmentProcessor::PipelineTextureSamplerRange(this->pipeline())) {
        SkASSERT(sampler.proxy()->isInstantiated());
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

    if (this->hasFixedPrimProcTextures()) {
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
