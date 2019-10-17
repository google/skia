/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrProgramInfo.h"

#include "src/gpu/GrStencilSettings.h"

GrStencilSettings GrProgramInfo::stencilSettings() const {
    GrStencilSettings stencil;

    if (this->pipeline().isStencilEnabled()) {
        stencil.reset(*this->pipeline().getUserStencil(),
                      this->pipeline().hasStencilClip(),
                      this->numStencilBits());
    }

    return stencil;
}

#ifdef SK_DEBUG
#include "src/gpu/GrMesh.h"
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

    if (this->hasDynamicPrimProcTextures()) {
        // Check that, for a given sampler, the properties of the dynamic textures remain
        // the same for all the meshes
        for (int s = 0; s < this->primProc().numTextureSamplers(); ++s) {
            auto dynamicPrimProcTextures = this->dynamicPrimProcTextures(0);

            const GrBackendFormat& format = dynamicPrimProcTextures[s]->backendFormat();
            GrTextureType type = dynamicPrimProcTextures[s]->textureType();
            GrPixelConfig config = dynamicPrimProcTextures[s]->config();

            for (int m = 1; m < fNumDynamicStateArrays; ++m) {
                dynamicPrimProcTextures = this->dynamicPrimProcTextures(m);

                auto testProxy = dynamicPrimProcTextures[s];
                SkASSERT(testProxy->backendFormat() == format);
                SkASSERT(testProxy->textureType() == type);
                SkASSERT(testProxy->config() == config);
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

    GrFragmentProcessor::Iter iter(this->pipeline());
    while (const GrFragmentProcessor* fp = iter.next()) {
        for (int s = 0; s < fp->numTextureSamplers(); ++s) {
            const auto& textureSampler = fp->textureSampler(s);
            assertResolved(textureSampler.peekTexture(), textureSampler.samplerState());
        }
    }
}

void GrProgramInfo::compatibleWithMeshes(const GrMesh meshes[], int meshCount) const {
    SkASSERT(!fNumDynamicStateArrays || meshCount == fNumDynamicStateArrays);

    for (int i = 0; i < meshCount; ++i) {
        SkASSERT(fPrimProc.hasVertexAttributes() == meshes[i].hasVertexData());
        SkASSERT(fPrimProc.hasInstanceAttributes() == meshes[i].hasInstanceData());
    }
}

#endif
