
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTexture.h"

#include "GrContext.h"
#include "GrGpu.h"
#include "GrRenderTarget.h"
#include "GrResourceCache.h"

SK_DEFINE_INST_COUNT(GrTexture)

/** 
 * This method allows us to interrupt the normal deletion process and place
 * textures back in the texture cache when their ref count goes to zero.
 */
void GrTexture::internal_dispose() const {

    if (this->isSetFlag((GrTextureFlags) kReturnToCache_FlagBit) &&
        NULL != this->INHERITED::getContext()) {
        GrTexture* nonConstThis = const_cast<GrTexture *>(this);
        this->fRefCnt = 1;      // restore ref count to initial setting

        nonConstThis->resetFlag((GrTextureFlags) kReturnToCache_FlagBit);
        nonConstThis->INHERITED::getContext()->addExistingTextureToCache(nonConstThis);

        // Note: "this" texture might be freed inside addExistingTextureToCache 
        // if it is purged.
        return;
    }

    this->INHERITED::internal_dispose();
}

bool GrTexture::readPixels(int left, int top, int width, int height,
                           GrPixelConfig config, void* buffer,
                           size_t rowBytes) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return false;
    }
    return context->readTexturePixels(this,
                                      left, top,
                                      width, height,
                                      config, buffer, rowBytes);
}

void GrTexture::writePixels(int left, int top, int width, int height,
                            GrPixelConfig config, const void* buffer,
                            size_t rowBytes) {
    // go through context so that all necessary flushing occurs
    GrContext* context = this->getContext();
    if (NULL == context) {
        return;
    }
    context->writeTexturePixels(this,
                                left, top,
                                width, height,
                                config, buffer, rowBytes);
}

void GrTexture::releaseRenderTarget() {
    if (NULL != fRenderTarget) {
        GrAssert(fRenderTarget->asTexture() == this);
        GrAssert(fDesc.fFlags & kRenderTarget_GrTextureFlagBit);

        fRenderTarget->onTextureReleaseRenderTarget();
        fRenderTarget->unref();
        fRenderTarget = NULL;

        fDesc.fFlags = fDesc.fFlags &
            ~(kRenderTarget_GrTextureFlagBit|kNoStencil_GrTextureFlagBit);
        fDesc.fSampleCnt = 0;
    }
}

void GrTexture::onRelease() {
    GrAssert(!this->isSetFlag((GrTextureFlags) kReturnToCache_FlagBit));
    this->releaseRenderTarget();
}

void GrTexture::onAbandon() {
    if (NULL != fRenderTarget) {
        fRenderTarget->abandon();
    }
}

void GrTexture::validateDesc() const {
    if (NULL != this->asRenderTarget()) {
        // This texture has a render target
        GrAssert(0 != (fDesc.fFlags & kRenderTarget_GrTextureFlagBit));

        if (NULL != this->asRenderTarget()->getStencilBuffer()) {
            GrAssert(0 != (fDesc.fFlags & kNoStencil_GrTextureFlagBit));
        } else {
            GrAssert(0 == (fDesc.fFlags & kNoStencil_GrTextureFlagBit));
        }

        GrAssert(fDesc.fSampleCnt == this->asRenderTarget()->numSamples());
    } else {
        GrAssert(0 == (fDesc.fFlags & kRenderTarget_GrTextureFlagBit));
        GrAssert(0 == (fDesc.fFlags & kNoStencil_GrTextureFlagBit));
        GrAssert(0 == fDesc.fSampleCnt);
    }
}

enum TextureBits {
    kFirst_TextureBit = (GrResourceKey::kLastPublic_TypeBit << 1),

    /*
     * The kNPOT bit is set when the texture is NPOT and is being repeated
     * but the hardware doesn't support that feature. 
     */
    kNPOT_TextureBit            = kFirst_TextureBit,
    /*
     * The kFilter bit can only be set when the kNPOT flag is set and indicates
     * whether the resizing of the texture should use filtering. This is
     * to handle cases where the original texture is indexed to disable
     * filtering.
     */
    kFilter_TextureBit          = kNPOT_TextureBit << 1,
    /*
     * The kScratch bit is set if the texture is being used as a scratch
     * texture.
     */
    kScratch_TextureBit         = kFilter_TextureBit << 1,
};

namespace {
void gen_texture_key_values(const GrGpu* gpu,
                            const GrSamplerState* sampler,
                            const GrTextureDesc& desc,
                            bool scratch,
                            uint32_t v[4]) {

    uint64_t clientKey = desc.fClientCacheID;

    if (scratch) {
        // Instead of a client-provided key of the texture contents
        // we create a key of from the descriptor.
        GrAssert(kScratch_CacheID == clientKey);
        clientKey = (desc.fFlags << 8) | ((uint64_t) desc.fConfig << 32);
    }

    // we assume we only need 16 bits of width and height
    // assert that texture creation will fail anyway if this assumption
    // would cause key collisions.
    GrAssert(gpu->getCaps().fMaxTextureSize <= SK_MaxU16);
    v[0] = (uint32_t) (clientKey & 0xffffffffUL);
    v[1] = (uint32_t) ((clientKey >> 32) & 0xffffffffUL);
    v[2] = desc.fWidth | (desc.fHeight << 16);

    v[3] = (desc.fSampleCnt << 24);
    GrAssert(desc.fSampleCnt >= 0 && desc.fSampleCnt < 256);

    if (!gpu->getCaps().fNPOTTextureTileSupport) {
        bool isPow2 = GrIsPow2(desc.fWidth) && GrIsPow2(desc.fHeight);

        bool tiled = NULL != sampler &&
                   ((sampler->getWrapX() != GrSamplerState::kClamp_WrapMode) ||
                    (sampler->getWrapY() != GrSamplerState::kClamp_WrapMode));

        if (tiled && !isPow2) {
            v[3] |= kNPOT_TextureBit;
            if (GrSamplerState::kNearest_Filter != sampler->getFilter()) {
                v[3] |= kFilter_TextureBit;
            }
        }
    }

    if (scratch) {
        v[3] |= kScratch_TextureBit;
    }

    v[3] |= GrResourceKey::kTexture_TypeBit;
}
}

GrResourceKey GrTexture::ComputeKey(const GrGpu* gpu,
                                    const GrSamplerState* sampler,
                                    const GrTextureDesc& desc,
                                    bool scratch) {
    uint32_t v[4];
    gen_texture_key_values(gpu, sampler, desc, scratch, v);
    return GrResourceKey(v);
}

bool GrTexture::NeedsResizing(const GrResourceKey& key) {
    return 0 != (key.getValue32(3) & kNPOT_TextureBit);
}

bool GrTexture::IsScratchTexture(const GrResourceKey& key) {
    return 0 != (key.getValue32(3) & kScratch_TextureBit);
}

bool GrTexture::NeedsFiltering(const GrResourceKey& key) {
    return 0 != (key.getValue32(3) & kFilter_TextureBit);
}
