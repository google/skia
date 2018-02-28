
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "GrBackendSurface.h"
#include "GrSamplerState.h"
#include "GrSurface.h"
#include "SkImage.h"
#include "SkPoint.h"
#include "SkRefCnt.h"
#include "../private/GrTypesPriv.h"

class GrTexturePriv;
enum class SkDestinationSurfaceColorMode;

class GrTexture : virtual public GrSurface {
public:
    GrTexture* asTexture() override { return this; }
    const GrTexture* asTexture() const override { return this; }

    /**
     *  Return the native ID or handle to the texture, depending on the
     *  platform. e.g. on OpenGL, return the texture ID.
     */
    virtual GrBackendObject getTextureHandle() const = 0;

    virtual GrBackendTexture getBackendTexture() const = 0;

    /**
     * This function indicates that the texture parameters (wrap mode, filtering, ...) have been
     * changed externally to Skia.
     */
    virtual void textureParamsModified() = 0;

    /**
     * This function steals the backend texture from a uniquely owned GrTexture with no pending
     * IO, passing it out to the caller. The GrTexture is deleted in the process.
     *
     * Note that if the GrTexture is not uniquely owned (no other refs), or has pending IO, this
     * function will fail.
     */
    static bool StealBackendTexture(sk_sp<GrTexture>&&,
                                    GrBackendTexture*,
                                    SkImage::BackendTextureReleaseProc*);

#ifdef SK_DEBUG
    void validate() const {
        this->INHERITED::validate();
    }
#endif

    virtual void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) = 0;

    // These match the definitions in SkImage, from whence they came.
    // TODO: Either move Chrome over to new api or remove their need to call this on GrTexture
    typedef void* ReleaseCtx;
    typedef void (*ReleaseProc)(ReleaseCtx);
    void setRelease(ReleaseProc proc, ReleaseCtx ctx) {
        sk_sp<GrReleaseProcHelper> helper(new GrReleaseProcHelper(proc, ctx));
        this->setRelease(std::move(helper));
    }

    /** Access methods that are only to be used within Skia code. */
    inline GrTexturePriv texturePriv();
    inline const GrTexturePriv texturePriv() const;

protected:
    GrTexture(GrGpu*, const GrSurfaceDesc&, GrSLType samplerType,
              GrSamplerState::Filter highestFilterMode, GrMipMapsStatus);

    virtual bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) = 0;

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
