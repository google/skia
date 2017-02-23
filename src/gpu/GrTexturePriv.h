/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexturePriv_DEFINED
#define GrTexturePriv_DEFINED

#include "GrExternalTextureData.h"
#include "GrTexture.h"

/** Class that adds methods to GrTexture that are only intended for use internal to Skia.
    This class is purely a privileged window into GrTexture. It should never have additional data
    members or virtual methods.
    Non-static methods that are not trivial inlines should be spring-boarded (e.g. declared and
    implemented privately in GrTexture with a inline public method here). */
class GrTexturePriv {
public:
    void setFlag(GrSurfaceFlags flags) {
        fTexture->fDesc.fFlags = fTexture->fDesc.fFlags | flags;
    }

    void resetFlag(GrSurfaceFlags flags) {
        fTexture->fDesc.fFlags = fTexture->fDesc.fFlags & ~flags;
    }

    bool isSetFlag(GrSurfaceFlags flags) const {
        return 0 != (fTexture->fDesc.fFlags & flags);
    }

    void dirtyMipMaps(bool mipMapsDirty) {
        fTexture->dirtyMipMaps(mipMapsDirty);
    }

    bool mipMapsAreDirty() const {
        return GrTexture::kValid_MipMapsStatus != fTexture->fMipMapsStatus;
    }

    bool hasMipMaps() const {
        return GrTexture::kNotAllocated_MipMapsStatus != fTexture->fMipMapsStatus;
    }

    void setMaxMipMapLevel(int maxMipMapLevel) const {
        fTexture->fMaxMipMapLevel = maxMipMapLevel;
    }

    int maxMipMapLevel() const {
        return fTexture->fMaxMipMapLevel;
    }

    GrSLType imageStorageType() const {
        if (GrPixelConfigIsSint(fTexture->config())) {
            return kIImageStorage2D_GrSLType;
        } else {
            return kImageStorage2D_GrSLType;
        }
    }

    GrSLType samplerType() const { return fTexture->fSamplerType; }

    /** The filter used is clamped to this value in GrProcessor::TextureSampler. */
    GrSamplerParams::FilterMode highestFilterMode() const { return fTexture->fHighestFilterMode; }

    void setMipColorMode(SkDestinationSurfaceColorMode colorMode) const {
        fTexture->fMipColorMode = colorMode;
    }
    SkDestinationSurfaceColorMode mipColorMode() const { return fTexture->fMipColorMode; }

    /**
     *  Return the native bookkeeping data for this texture, and detach the backend object from
     *  this GrTexture. It's lifetime will no longer be managed by Ganesh, and this GrTexture will
     *  no longer refer to it. Leaves this GrTexture in an orphan state.
     */
    std::unique_ptr<GrExternalTextureData> detachBackendTexture() {
        return fTexture->detachBackendTexture();
    }

    static void ComputeScratchKey(const GrSurfaceDesc&, GrScratchKey*);

private:
    GrTexturePriv(GrTexture* texture) : fTexture(texture) { }
    GrTexturePriv(const GrTexturePriv& that) : fTexture(that.fTexture) { }
    GrTexturePriv& operator=(const GrTexturePriv&); // unimpl

    // No taking addresses of this type.
    const GrTexturePriv* operator&() const;
    GrTexturePriv* operator&();

    GrTexture* fTexture;

    friend class GrTexture; // to construct/copy this type.
};

inline GrTexturePriv GrTexture::texturePriv() { return GrTexturePriv(this); }

inline const GrTexturePriv GrTexture::texturePriv () const {
    return GrTexturePriv(const_cast<GrTexture*>(this));
}

#endif
