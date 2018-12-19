
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

class SK_API GrTexture : virtual public GrSurface {
public:
    GrTexture* asTexture() override { return this; }
    const GrTexture* asTexture() const override { return this; }

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

    /**
     * Installs a proc on this texture. It will be called when the texture becomes "idle". Idle is
     * defined to mean that the texture has no refs or pending IOs and that GPU I/O operations on
     * the texture are completed if the backend API disallows deletion of a texture before such
     * operations occur (e.g. Vulkan). After the idle proc is called it is removed. The idle proc
     * will always be called before the texture is destroyed, even in unusual shutdown scenarios
     * (e.g. GrContext::abandonContext()).
     */
    using IdleProc = void(void*);
    virtual void setIdleProc(IdleProc, void* context) = 0;

    /** Access methods that are only to be used within Skia code. */
    inline GrTexturePriv texturePriv();
    inline const GrTexturePriv texturePriv() const;

protected:
    GrTexture(GrGpu*, const GrSurfaceDesc&, GrTextureType, GrMipMapsStatus);

    virtual bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) = 0;

private:
    void computeScratchKey(GrScratchKey*) const override;
    size_t onGpuMemorySize() const override;
    void markMipMapsDirty();
    void markMipMapsClean();

    GrTextureType                 fTextureType;
    GrMipMapsStatus               fMipMapsStatus;
    int                           fMaxMipMapLevel;
    friend class GrTexturePriv;

    typedef GrSurface INHERITED;
};

#endif
