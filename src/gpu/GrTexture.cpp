/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrResourceKey.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"
#include "GrTypes.h"
#include "SkMath.h"
#include "SkMipMap.h"
#include "SkTypes.h"

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
            // TODO(http://skbug.com/4548) - The desc and scratch key should be
            // updated to reflect the newly-allocated mipmaps.
        }
    }
}

size_t GrTexture::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(), 1,
                                  this->texturePriv().hasMipMaps(), false);
}

void GrTexture::validateDesc() const {
    if (this->asRenderTarget()) {
        // This texture has a render target
        SkASSERT(0 != (fDesc.fFlags & kRenderTarget_GrSurfaceFlag));
        SkASSERT(fDesc.fSampleCnt == this->asRenderTarget()->numColorSamples());
    } else {
        SkASSERT(0 == (fDesc.fFlags & kRenderTarget_GrSurfaceFlag));
        SkASSERT(0 == fDesc.fSampleCnt);
    }
}

//////////////////////////////////////////////////////////////////////////////

namespace {

// FIXME:  This should be refactored with the code in gl/GrGLGpu.cpp.
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
GrTexture::GrTexture(GrGpu* gpu, const GrSurfaceDesc& desc, GrSLType samplerType,
                     GrSamplerParams::FilterMode highestFilterMode, bool wasMipMapDataProvided)
    : INHERITED(gpu, desc)
    , fSamplerType(samplerType)
    , fHighestFilterMode(highestFilterMode)
    // Mip color mode is explicitly set after creation via GrTexturePriv
    , fMipColorMode(SkDestinationSurfaceColorMode::kLegacy) {
    if (wasMipMapDataProvided) {
        fMipMapsStatus = kValid_MipMapsStatus;
        fMaxMipMapLevel = SkMipMap::ComputeLevelCount(fDesc.fWidth, fDesc.fHeight);
    } else {
        fMipMapsStatus = kNotAllocated_MipMapsStatus;
        fMaxMipMapLevel = 0;
    }
}

void GrTexture::computeScratchKey(GrScratchKey* key) const {
    if (!GrPixelConfigIsCompressed(fDesc.fConfig)) {
        GrTexturePriv::ComputeScratchKey(fDesc, key);
    }
}

void GrTexturePriv::ComputeScratchKey(const GrSurfaceDesc& desc, GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();

    GrSurfaceOrigin origin = resolve_origin(desc);
    uint32_t flags = desc.fFlags;

    // make sure desc.fConfig fits in 5 bits
    SkASSERT(sk_float_log2(kLast_GrPixelConfig) <= 5);
    SkASSERT(static_cast<int>(desc.fConfig) < (1 << 5));
    SkASSERT(desc.fSampleCnt < (1 << 8));
    SkASSERT(flags < (1 << 10));
    SkASSERT(static_cast<int>(origin) < (1 << 8));

    GrScratchKey::Builder builder(key, kType, 3);
    builder[0] = desc.fWidth;
    builder[1] = desc.fHeight;
    builder[2] = desc.fConfig | (desc.fIsMipMapped << 5) | (desc.fSampleCnt << 6) | (flags << 14)
                 | (origin << 24);
}
