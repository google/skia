/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProgramInfo.h"


#ifdef SK_DEBUG
#include "src/gpu/GrTexturePriv.h"

void GrProgramInfo::validate() const {
    SkASSERT(!fPipeline.isBad());

    if (this->hasDynamicPrimProcTextures()) {
        SkASSERT(!this->hasFixedPrimProcTextures());
        SkASSERT(fPrimProc.numTextureSamplers());
    } else if (this->hasFixedPrimProcTextures()) {
        SkASSERT(fPrimProc.numTextureSamplers());
    } else {
        SkASSERT(!fPrimProc.numTextureSamplers());
    }

    SkASSERT(!fPipeline.isScissorEnabled() || this->hasFixedScissor() ||
             this->hasDynamicScissors());
}

void GrProgramInfo::checkAllInstantiated(int meshCount) const {
    if (this->hasFixedPrimProcTextures()) {
        auto fixedPrimProcTextures = this->fixedPrimProcTextures();
        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            SkASSERT(fixedPrimProcTextures[s]->isInstantiated());
        }
    }

    if (this->hasDynamicPrimProcTextures()) {
        for (int m = 0; m < meshCount; ++m) {
            auto dynamicPrimProcTextures = this->dynamicPrimProcTextures(m);
            for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
                SkASSERT(dynamicPrimProcTextures[s]->isInstantiated());
            }
        }

        // TODO: if GrProgramInfo had the mesh count we could do this in validate!
        // Check that, for a given sampler, the properties of the dynamic textures remain
        // the same for all the meshes
        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            auto dynamicPrimProcTextures = this->dynamicPrimProcTextures(0);

            const GrBackendFormat& format = dynamicPrimProcTextures[s]->backendFormat();
            GrTextureType type = dynamicPrimProcTextures[s]->textureType();
            GrPixelConfig config = dynamicPrimProcTextures[s]->config();

            for (int m = 1; m < meshCount; ++m) {
                dynamicPrimProcTextures = this->dynamicPrimProcTextures(m);

                auto testProxy = dynamicPrimProcTextures[s];
                SkASSERT(testProxy->backendFormat() == format);
                SkASSERT(testProxy->textureType() == type);
                SkASSERT(testProxy->config() == config);
            }
        }
    }
}

void GrProgramInfo::checkMSAAAndMIPSAreResolved(int meshCount) const {

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

    if (this->hasDynamicPrimProcTextures()) {
        for (int m = 0; m < meshCount; ++m) {
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

    GrFragmentProcessor::Iter iter(this->pipeline());
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int s = 0; s < fp->numTextureSamplers(); ++s) {
            const auto& textureSampler = fp->textureSampler(s);
            assertResolved(textureSampler.peekTexture(), textureSampler.samplerState());
        }
    }
}

#endif
