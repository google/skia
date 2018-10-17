/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "GrGpu.h"
#include "GrTexture.h"
#include "GrGLUtil.h"

class GrGLGpu;

class GrGLTexture : public GrTexture {
public:
    // Texture state that overlaps with sampler object state.
    struct SamplerParams {
        GrGLenum fMinFilter;
        GrGLenum fMagFilter;
        GrGLenum fWrapS;
        GrGLenum fWrapT;
        GrGLenum fMaxMipMapLevel;
        void invalidate() { memset(this, 0xff, sizeof(SamplerParams)); }
    };

    struct IDDesc {
        GrGLTextureInfo             fInfo;
        GrBackendObjectOwnership    fOwnership;
    };

    static GrTextureType TextureTypeFromTarget(GrGLenum textureTarget);

    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    ~GrGLTexture() override {
        // check that invokeReleaseProc has been called (if needed)
        SkASSERT(!fReleaseHelper);
    }

    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override {
        fSamplerParams.invalidate();
        fCachedSwizzleKey = ~0U;
    }

    void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {
        fReleaseHelper = std::move(releaseHelper);
    }

    // These functions are used to track the texture parameters associated with the texture.
    GrGpu::ResetTimestamp getCachedParamsTimestamp() const { return fParamsTimestamp; }
    const SamplerParams& getCachedSamplerParams() const { return fSamplerParams; }
    uint32_t getCacheSwizzleKey() const { return fCachedSwizzleKey; }

    void setCachedParams(const SamplerParams* samplerParams, const GrSwizzle& swizzle,
                         GrGpu::ResetTimestamp currTimestamp) {
        if (samplerParams) {
            fSamplerParams = *samplerParams;
        }
        fCachedSwizzleKey = swizzle.asKey();
        fParamsTimestamp = currTimestamp;
    }

    GrGLuint textureID() const { return fID; }

    GrGLenum target() const;

    bool hasBaseLevelBeenBoundToFBO() const { return fBaseLevelHasBeenBoundToFBO; }
    void baseLevelWasBoundToFBO() { fBaseLevelHasBeenBoundToFBO = true; }

    static sk_sp<GrGLTexture> MakeWrapped(GrGLGpu*, const GrSurfaceDesc&, GrMipMapsStatus,
                                          const IDDesc&);

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    enum Wrapped { kWrapped };
    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*, Wrapped, const GrSurfaceDesc&, GrMipMapsStatus, const IDDesc&);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override;

private:
    void invokeReleaseProc() {
        if (fReleaseHelper) {
            // Depending on the ref count of fReleaseHelper this may or may not actually trigger the
            // ReleaseProc to be called.
            fReleaseHelper.reset();
        }
    }

    SamplerParams fSamplerParams;
    GrGpu::ResetTimestamp fParamsTimestamp;
    sk_sp<GrReleaseProcHelper> fReleaseHelper;
    uint32_t fCachedSwizzleKey;
    GrGLuint fID;
    GrGLenum fFormat;
    GrBackendObjectOwnership fTextureIDOwnership;
    bool fBaseLevelHasBeenBoundToFBO = false;

    typedef GrTexture INHERITED;
};

#endif
