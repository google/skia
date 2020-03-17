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
}

void GrProgramInfo::checkAllInstantiated() const {
    for (auto [sampler, fp] : GrFragmentProcessor::PipelineTextureSamplerRange(this->pipeline())) {
        SkASSERT(sampler.proxy()->isInstantiated());
    }
}

void GrProgramInfo::checkMSAAAndMIPSAreResolved() const {
    for (auto [sampler, fp] : GrFragmentProcessor::PipelineTextureSamplerRange(this->pipeline())) {
        GrTexture* tex = sampler.peekTexture();
        SkASSERT(tex);

        // Ensure mipmaps were all resolved ahead of time by the DAG.
        if (GrSamplerState::Filter::kMipMap == sampler.samplerState().filter() &&
            (tex->width() != 1 || tex->height() != 1)) {
            // There are some cases where we might be given a non-mipmapped texture with a mipmap
            // filter. See skbug.com/7094.
            SkASSERT(tex->texturePriv().mipMapped() != GrMipMapped::kYes ||
                     !tex->texturePriv().mipMapsAreDirty());
        }
    }
}

#endif
