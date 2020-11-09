
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
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrSurface.h"

class GrTexture : virtual public GrSurface {
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

    /**
     * Installs a proc on this texture. It will be called when the texture becomes "idle". "Idle"
     * means, accounting only for Skia's use of the texture, it is safe to delete in the underlying
     * API. This is used to implement release procs for promise image textures because we cache
     * the GrTexture object and thus can't rely on it's destructor to trigger a normal release proc.
     */
    virtual void addIdleProc(sk_sp<GrRefCntedCallback> idleProc) {
        // This is the default implementation for the managed case where the IdleState can be
        // ignored. Unmanaged backends, e.g. Vulkan, must override this to detect when the GPU
        // is finished accessing the texture.
        fIdleProcs.push_back(std::move(idleProc));
    }
    /** Helper version of addIdleProc that creates the ref-counted wrapper. */
    void addIdleProc(GrRefCntedCallback::Callback callback, GrRefCntedCallback::Context context) {
        this->addIdleProc(GrRefCntedCallback::Make(callback, context));
    }

    GrTextureType textureType() const { return fTextureType; }
    bool hasRestrictedSampling() const {
        return GrTextureTypeHasRestrictedSampling(this->textureType());
    }

    void markMipmapsDirty();
    void markMipmapsClean();
    GrMipmapped mipmapped() const {
        return GrMipmapped(fMipmapStatus != GrMipmapStatus::kNotAllocated);
    }
    bool mipmapsAreDirty() const { return fMipmapStatus != GrMipmapStatus::kValid; }
    GrMipmapStatus mipmapStatus() const { return fMipmapStatus; }
    int maxMipmapLevel() const { return fMaxMipmapLevel; }

    static void ComputeScratchKey(const GrCaps& caps,
                                  const GrBackendFormat& format,
                                  SkISize dimensions,
                                  GrRenderable,
                                  int sampleCnt,
                                  GrMipmapped,
                                  GrProtected,
                                  GrScratchKey* key);

protected:
    GrTexture(GrGpu*, const SkISize&, GrProtected, GrTextureType, GrMipmapStatus);

    virtual bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) = 0;

    SkTArray<sk_sp<GrRefCntedCallback>> fIdleProcs;

    void willRemoveLastRef() override {
        // We're about to be idle in the resource cache. Do our part to trigger the idle callbacks.
        fIdleProcs.reset();
    }
    virtual void callIdleProcsOnBehalfOfResource() {}
    void computeScratchKey(GrScratchKey*) const override;

private:
    size_t onGpuMemorySize() const override;

    GrTextureType                 fTextureType;
    GrMipmapStatus                fMipmapStatus;
    int                           fMaxMipmapLevel;
    friend class GrTextureResource;

    using INHERITED = GrSurface;
};

#endif
