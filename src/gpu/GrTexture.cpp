
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrResourceCache.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"

void GrTexture::dirtyMipMaps(bool mipMapsDirty) {
    if (mipMapsDirty) {
        if (kValid_MipMapsStatus == fMipMapsStatus) {
           fMipMapsStatus = kAllocated_MipMapsStatus;
        }
    } else {
        const bool sizeChanged = kNotAllocated_MipMapsStatus == fMipMapsStatus;
        fMipMapsStatus = kValid_MipMapsStatus;
        if (sizeChanged) {
            // This must not be called until after changing fMipMapsStatus.
            this->didChangeGpuMemorySize();
        }
    }
}

size_t GrTexture::gpuMemorySize() const {
    size_t textureSize;

    if (GrPixelConfigIsCompressed(fDesc.fConfig)) {
        textureSize = GrCompressedFormatDataSize(fDesc.fConfig, fDesc.fWidth, fDesc.fHeight);
    } else {
        textureSize = (size_t) fDesc.fWidth * fDesc.fHeight * GrBytesPerPixel(fDesc.fConfig);
    }

    if (this->texturePriv().hasMipMaps()) {
        // We don't have to worry about the mipmaps being a different size than
        // we'd expect because we never change fDesc.fWidth/fHeight.
        textureSize *= 2;
    }
    return textureSize;
}

void GrTexture::validateDesc() const {
    if (this->asRenderTarget()) {
        // This texture has a render target
        SkASSERT(0 != (fDesc.fFlags & kRenderTarget_GrSurfaceFlag));

        if (this->asRenderTarget()->getStencilBuffer()) {
            SkASSERT(0 != (fDesc.fFlags & kNoStencil_GrSurfaceFlag));
        } else {
            SkASSERT(0 == (fDesc.fFlags & kNoStencil_GrSurfaceFlag));
        }

        SkASSERT(fDesc.fSampleCnt == this->asRenderTarget()->numSamples());
    } else {
        SkASSERT(0 == (fDesc.fFlags & kRenderTarget_GrSurfaceFlag));
        SkASSERT(0 == (fDesc.fFlags & kNoStencil_GrSurfaceFlag));
        SkASSERT(0 == fDesc.fSampleCnt);
    }
}

//////////////////////////////////////////////////////////////////////////////

// These flags need to fit in a GrResourceKey::ResourceFlags so they can be folded into the texture
// key
enum TextureFlags {
    /**
     * The kStretchToPOT bit is set when the texture is NPOT and is being repeated but the
     * hardware doesn't support that feature.
     */
    kStretchToPOT_TextureFlag = 0x1,
    /**
     * The kBilerp bit can only be set when the kStretchToPOT flag is set and indicates whether the
     * stretched texture should be bilerped.
     */
     kBilerp_TextureFlag       = 0x2,
};

namespace {
GrResourceKey::ResourceFlags get_texture_flags(const GrGpu* gpu,
                                               const GrTextureParams* params,
                                               const GrSurfaceDesc& desc) {
    GrResourceKey::ResourceFlags flags = 0;
    bool tiled = params && params->isTiled();
    if (tiled && !gpu->caps()->npotTextureTileSupport()) {
        if (!SkIsPow2(desc.fWidth) || !SkIsPow2(desc.fHeight)) {
            flags |= kStretchToPOT_TextureFlag;
            switch(params->filterMode()) {
                case GrTextureParams::kNone_FilterMode:
                    break;
                case GrTextureParams::kBilerp_FilterMode:
                case GrTextureParams::kMipMap_FilterMode:
                    flags |= kBilerp_TextureFlag;
                    break;
            }
        }
    }
    return flags;
}

// FIXME:  This should be refactored with the code in gl/GrGpuGL.cpp.
GrSurfaceOrigin resolve_origin(const GrSurfaceDesc& desc) {
    // By default, GrRenderTargets are GL's normal orientation so that they
    // can be drawn to by the outside world without the client having
    // to render upside down.
    bool renderTarget = 0 != (desc.fFlags & kRenderTarget_GrSurfaceFlag);
    if (kDefault_GrSurfaceOrigin == desc.fOrigin) {
        return renderTarget ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;
    } else {
        return desc.fOrigin;
    }
}
}

//////////////////////////////////////////////////////////////////////////////
GrTexture::GrTexture(GrGpu* gpu, bool isWrapped, const GrSurfaceDesc& desc)
    : INHERITED(gpu, isWrapped, desc)
    , fMipMapsStatus(kNotAllocated_MipMapsStatus) {
    this->setScratchKey(GrTexturePriv::ComputeScratchKey(desc));
    // only make sense if alloc size is pow2
    fShiftFixedX = 31 - SkCLZ(fDesc.fWidth);
    fShiftFixedY = 31 - SkCLZ(fDesc.fHeight);
}

GrResourceKey GrTexturePriv::ComputeKey(const GrGpu* gpu,
                                    const GrTextureParams* params,
                                    const GrSurfaceDesc& desc,
                                    const GrCacheID& cacheID) {
    GrResourceKey::ResourceFlags flags = get_texture_flags(gpu, params, desc);
    return GrResourceKey(cacheID, ResourceType(), flags);
}

GrResourceKey GrTexturePriv::ComputeScratchKey(const GrSurfaceDesc& desc) {
    GrCacheID::Key idKey;
    // Instead of a client-provided key of the texture contents we create a key from the
    // descriptor.
    GR_STATIC_ASSERT(sizeof(idKey) >= 16);
    SkASSERT(desc.fHeight < (1 << 16));
    SkASSERT(desc.fWidth < (1 << 16));
    idKey.fData32[0] = (desc.fWidth) | (desc.fHeight << 16);
    idKey.fData32[1] = desc.fConfig | desc.fSampleCnt << 16;
    idKey.fData32[2] = desc.fFlags;
    idKey.fData32[3] = resolve_origin(desc);    // Only needs 2 bits actually
    static const int kPadSize = sizeof(idKey) - 16;
    GR_STATIC_ASSERT(kPadSize >= 0);
    memset(idKey.fData8 + 16, 0, kPadSize);

    GrCacheID cacheID(GrResourceKey::ScratchDomain(), idKey);
    return GrResourceKey(cacheID, ResourceType(), 0);
}

bool GrTexturePriv::NeedsResizing(const GrResourceKey& key) {
    return SkToBool(key.getResourceFlags() & kStretchToPOT_TextureFlag);
}

bool GrTexturePriv::NeedsBilerp(const GrResourceKey& key) {
    return SkToBool(key.getResourceFlags() & kBilerp_TextureFlag);
}
