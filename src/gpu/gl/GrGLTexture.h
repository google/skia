/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "include/private/GrGLTypesPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrTexture.h"
#include "src/gpu/gl/GrGLUtil.h"

class GrGLGpu;

class GrGLTexture : public GrTexture {
public:
    struct Desc {
        SkISize fSize                       = {-1, -1};
        GrGLenum fTarget                    = 0;
        GrGLuint fID                        = 0;
        GrGLFormat fFormat                  = GrGLFormat::kUnknown;
        GrBackendObjectOwnership fOwnership = GrBackendObjectOwnership::kOwned;
    };

    static GrTextureType TextureTypeFromTarget(GrGLenum textureTarget);

    GrGLTexture(GrGLGpu*, SkBudgeted, const Desc&, GrMipMapsStatus);

    ~GrGLTexture() override {}

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    // TODO: Remove once clients are no longer calling this.
    void textureParamsModified() override { fParameters->invalidate(); }

    GrGLTextureParameters* parameters() { return fParameters.get(); }

    GrGLuint textureID() const { return fID; }

    GrGLenum target() const;

    GrGLFormat format() const { return fFormat; }

    bool hasBaseLevelBeenBoundToFBO() const { return fBaseLevelHasBeenBoundToFBO; }
    void baseLevelWasBoundToFBO() { fBaseLevelHasBeenBoundToFBO = true; }

    static sk_sp<GrGLTexture> MakeWrapped(GrGLGpu*,
                                          GrMipMapsStatus,
                                          const Desc&,
                                          sk_sp<GrGLTextureParameters>,
                                          GrWrapCacheable, GrIOType);

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*, const Desc&, sk_sp<GrGLTextureParameters>, GrMipMapsStatus);

    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*,
                const Desc&,
                GrMipMapsStatus,
                sk_sp<GrGLTextureParameters>,
                GrWrapCacheable,
                GrIOType);

    void init(const Desc&);

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override;

private:
    sk_sp<GrGLTextureParameters> fParameters;
    GrGLuint fID;
    GrGLFormat fFormat;
    GrBackendObjectOwnership fTextureIDOwnership;
    bool fBaseLevelHasBeenBoundToFBO = false;

    typedef GrTexture INHERITED;
};

#endif
