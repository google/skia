
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "GrSamplerState.h"
#include "GrSurface.h"
#include "SkPoint.h"
#include "SkRefCnt.h"

class GrTexturePriv;

class GrTexture : virtual public GrSurface {
public:
    GrTexture* asTexture() override { return this; }
    const GrTexture* asTexture() const override { return this; }

    /**
     *  Return the native ID or handle to the texture, depending on the
     *  platform. e.g. on OpenGL, return the texture ID.
     */
    virtual GrBackendObject getTextureHandle() const = 0;

    /**
     * This function indicates that the texture parameters (wrap mode, filtering, ...) have been
     * changed externally to Skia.
     */
    virtual void textureParamsModified() = 0;

#ifdef SK_DEBUG
    void validate() const {
        this->INHERITED::validate();
    }
#endif

    // These match the definitions in SkImage, for whence they came
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);

    virtual void setRelease(ReleaseProc proc, ReleaseCtx ctx) = 0;

    /** Access methods that are only to be used within Skia code. */
    inline GrTexturePriv texturePriv();
    inline const GrTexturePriv texturePriv() const;

protected:
    // TODO: Once we disable support for mip maps on textures which were not allocated with them at
    // creation, we can check the highestFilterMode for mip map to see if mip maps were allocated.
    // Until then we need to explicitly pass in the mipsAllocated bool.
    GrTexture(GrGpu*, const GrSurfaceDesc&, GrSLType samplerType,
              GrSamplerState::Filter highestFilterMode, bool mipsAllocated,
              bool wasFullMipMapDataProvided);

private:
    void computeScratchKey(GrScratchKey*) const override;
    size_t onGpuMemorySize() const override;
    void dirtyMipMaps(bool mipMapsDirty);

    enum MipMapsStatus {
        kNotAllocated_MipMapsStatus, // Mips have not been allocated
        kInvalid_MipMapsStatus,      // Mips have been allocated but nothing written to base level
        kDirty_MipMapsStatus,        // Base level has data but the rest of the levels are dirty
        kClean_MipMapsStatus         // All levels fully allocated and have valid data in them
    };

    GrSLType                      fSamplerType;
    GrSamplerState::Filter        fHighestFilterMode;
    MipMapsStatus                 fMipMapsStatus;
    int                           fMaxMipMapLevel;
    SkDestinationSurfaceColorMode fMipColorMode;
    friend class GrTexturePriv;

    typedef GrSurface INHERITED;
};

#endif
