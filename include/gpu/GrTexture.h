
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTexture_DEFINED
#define GrTexture_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrSamplerState.h"
#include "include/gpu/GrSurface.h"
#include "include/private/GrTypesPriv.h"

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
    static bool StealBackendTexture(sk_sp<GrTexture>,
                                    GrBackendTexture*,
                                    SkImage::BackendTextureReleaseProc*);

#ifdef SK_DEBUG
    void validate() const {
        this->INHERITED::validate();
    }
#endif

    /** See addIdleProc. */
    enum class IdleState {
        kFlushed,
        kFinished
    };
    /**
     * Installs a proc on this texture. It will be called when the texture becomes "idle". There
     * are two types of idle states as indicated by IdleState. For managed backends (e.g. GL where
     * a driver typically handles CPU/GPU synchronization of resource access) there is no difference
     * between the two. They both mean "all work related to the resource has been flushed to the
     * backend API and the texture is not owned outside the resource cache".
     *
     * If the API is unmanaged (e.g. Vulkan) then kFinished has the additional constraint that the
     * work flushed to the GPU is finished.
     */
    virtual void addIdleProc(sk_sp<GrRefCntedCallback> idleProc, IdleState) {
        // This is the default implementation for the managed case where the IdleState can be
        // ignored. Unmanaged backends, e.g. Vulkan, must override this to consider IdleState.
        fIdleProcs.push_back(std::move(idleProc));
    }
    /** Helper version of addIdleProc that creates the ref-counted wrapper. */
    void addIdleProc(GrRefCntedCallback::Callback callback,
                     GrRefCntedCallback::Context context,
                     IdleState state) {
        this->addIdleProc(sk_make_sp<GrRefCntedCallback>(callback, context), state);
    }

    /** Access methods that are only to be used within Skia code. */
    inline GrTexturePriv texturePriv();
    inline const GrTexturePriv texturePriv() const;

protected:
    GrTexture(GrGpu*, const GrSurfaceDesc&, GrProtected, GrTextureType, GrMipMapsStatus);

    virtual bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) = 0;

    SkTArray<sk_sp<GrRefCntedCallback>> fIdleProcs;

    void willRemoveLastRefOrPendingIO() override {
        // We're about to be idle in the resource cache. Do our part to trigger the idle callbacks.
        fIdleProcs.reset();
    }

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
