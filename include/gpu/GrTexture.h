
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
#include "../private/GrTypesPriv.h"

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
    GrTexture(GrGpu*, const GrSurfaceDesc&, GrSLType samplerType,
              GrSamplerState::Filter highestFilterMode, GrMipMapsStatus);

private:
    void computeScratchKey(GrScratchKey*) const override;
    size_t onGpuMemorySize() const override;
    void markMipMapsDirty();
    void markMipMapsClean();

    GrSLType                      fSamplerType;
    GrSamplerState::Filter        fHighestFilterMode;
    GrMipMapsStatus               fMipMapsStatus;
    int                           fMaxMipMapLevel;
    SkDestinationSurfaceColorMode fMipColorMode;
    friend class GrTexturePriv;

    typedef GrSurface INHERITED;
};

#endif
