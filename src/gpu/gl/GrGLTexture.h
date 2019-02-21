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
    // Texture state that overlaps with sampler object state. We don't need to track this if we
    // are using sampler objects.
    struct SamplerParams {
        // These are the OpenGL defaults.
        GrGLenum fMinFilter = GR_GL_NEAREST_MIPMAP_LINEAR;
        GrGLenum fMagFilter = GR_GL_LINEAR;
        GrGLenum fWrapS = GR_GL_REPEAT;
        GrGLenum fWrapT = GR_GL_REPEAT;
        GrGLfloat fMinLOD = -1000.f;
        GrGLfloat fMaxLOD = 1000.f;
        // We always want the border color to be transparent black, so no need to store 4 floats.
        // Just track if it's been invalidated and no longer the default
        bool fBorderColorInvalid = false;

        void invalidate() {
            fMinFilter = ~0U;
            fMagFilter = ~0U;
            fWrapS = ~0U;
            fWrapT = ~0U;
            fMinLOD = SK_ScalarNaN;
            fMaxLOD = SK_ScalarNaN;
            fBorderColorInvalid = true;
        }
    };

    // Texture state that does not overlap with sampler object state.
    struct NonSamplerParams {
        // These are the OpenGL defaults.
        uint32_t fSwizzleKey = GrSwizzle::RGBA().asKey();
        GrGLint fBaseMipMapLevel = 0;
        GrGLint fMaxMipMapLevel = 1000;
        void invalidate() {
            fSwizzleKey = ~0U;
            fBaseMipMapLevel = ~0;
            fMaxMipMapLevel = ~0;
        }
    };

    struct IDDesc {
        GrGLTextureInfo             fInfo;
        GrBackendObjectOwnership    fOwnership;
    };

    static GrTextureType TextureTypeFromTarget(GrGLenum textureTarget);

    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    ~GrGLTexture() override {}

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {
        fSamplerParams.invalidate();
        fNonSamplerParams.invalidate();
    }

    void setIdleProc(IdleProc proc, void* context) override {
        fIdleProc = proc;
        fIdleProcContext = context;
    }
    void* idleContext() const override { return fIdleProcContext; }

    // These functions are used to track the texture parameters associated with the texture.
    GrGpu::ResetTimestamp getCachedParamsTimestamp() const { return fParamsTimestamp; }
    const SamplerParams& getCachedSamplerParams() const { return fSamplerParams; }
    const NonSamplerParams& getCachedNonSamplerParams() const { return fNonSamplerParams; }

    void setCachedParams(const SamplerParams* samplerParams,
                         const NonSamplerParams& nonSamplerParams,
                         GrGpu::ResetTimestamp currTimestamp) {
        if (samplerParams) {
            fSamplerParams = *samplerParams;
        }
        fNonSamplerParams = nonSamplerParams;
        fParamsTimestamp = currTimestamp;
    }

    GrGLuint textureID() const { return fID; }

    GrGLenum target() const;

    bool hasBaseLevelBeenBoundToFBO() const { return fBaseLevelHasBeenBoundToFBO; }
    void baseLevelWasBoundToFBO() { fBaseLevelHasBeenBoundToFBO = true; }

    static sk_sp<GrGLTexture> MakeWrapped(GrGLGpu*, const GrSurfaceDesc&, GrMipMapsStatus,
                                          const IDDesc&, GrWrapCacheable, GrIOType);

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, GrMipMapsStatus, const IDDesc&, GrWrapCacheable,
                GrIOType);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override;

private:
    void onSetRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {}

    void willRemoveLastRefOrPendingIO() override {
        if (fIdleProc) {
            fIdleProc(fIdleProcContext);
            fIdleProc = nullptr;
            fIdleProcContext = nullptr;
        }
    }

    SamplerParams fSamplerParams;
    NonSamplerParams fNonSamplerParams;
    GrGpu::ResetTimestamp fParamsTimestamp;
    IdleProc* fIdleProc = nullptr;
    void* fIdleProcContext = nullptr;
    GrGLuint fID;
    GrGLenum fFormat;
    GrBackendObjectOwnership fTextureIDOwnership;
    bool fBaseLevelHasBeenBoundToFBO = false;

    typedef GrTexture INHERITED;
};

#endif
