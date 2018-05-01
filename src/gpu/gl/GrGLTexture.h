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
    struct TexParams {
        GrGLenum fMinFilter;
        GrGLenum fMagFilter;
        GrGLenum fWrapS;
        GrGLenum fWrapT;
        GrGLenum fMaxMipMapLevel;
        GrGLenum fSwizzleRGBA[4];
        GrGLenum fSRGBDecode;
        void invalidate() { memset(this, 0xff, sizeof(TexParams)); }
    };

    struct IDDesc {
        GrGLTextureInfo             fInfo;
        GrBackendObjectOwnership    fOwnership;
    };
    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    ~GrGLTexture() override {
        // check that invokeReleaseProc has been called (if needed)
        SkASSERT(!fReleaseHelper);
    }

    GrBackendObject getTextureHandle() const override;
    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override { fTexParams.invalidate(); }

    void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {
        fReleaseHelper = std::move(releaseHelper);
    }

    // These functions are used to track the texture parameters associated with the texture.
    const TexParams& getCachedTexParams(GrGpu::ResetTimestamp* timestamp) const {
        *timestamp = fTexParamsTimestamp;
        return fTexParams;
    }

    void setCachedTexParams(const TexParams& texParams,
                            GrGpu::ResetTimestamp timestamp) {
        fTexParams = texParams;
        fTexParamsTimestamp = timestamp;
    }

    GrGLuint textureID() const { return fInfo.fID; }

    GrGLenum target() const { return fInfo.fTarget; }

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

    TexParams                       fTexParams;
    GrGpu::ResetTimestamp           fTexParamsTimestamp;
    // Holds the texture target and ID. A pointer to this may be shared to external clients for
    // direct interaction with the GL object.
    GrGLTextureInfo                 fInfo;
    GrBackendObjectOwnership        fTextureIDOwnership;
    bool                            fBaseLevelHasBeenBoundToFBO = false;

    sk_sp<GrReleaseProcHelper>      fReleaseHelper;

    typedef GrTexture INHERITED;
};

#endif
