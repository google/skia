/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexturePriv_DEFINED
#define GrTexturePriv_DEFINED

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

    void dirtyMipMaps(bool mipMapsDirty) { fTexture->dirtyMipMaps(mipMapsDirty); }

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
