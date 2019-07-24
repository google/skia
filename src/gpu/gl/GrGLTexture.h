/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "include/gpu/GrTexture.h"
#include "include/private/GrGLTypesPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/gl/GrGLUtil.h"

class GrGLGpu;

class GrGLTexture : public GrTexture {
public:
    struct IDDesc {
        GrGLTextureInfo             fInfo;
        GrBackendObjectOwnership    fOwnership;
    };

    static GrTextureType TextureTypeFromTarget(GrGLenum textureTarget);

    GrGLTexture(GrGLGpu*, SkBudgeted, const GrSurfaceDesc&, const IDDesc&, GrMipMapsStatus);

    ~GrGLTexture() override {}

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    // TODO: Remove once clients are no longer calling this.
    void textureParamsModified() override { fParameters->invalidate(); }

    GrGLTextureParameters* parameters() { return fParameters.get(); }

    GrGLuint textureID() const { return fID; }

    GrGLenum target() const;

    GrGLFormat format() const { return GrGLFormatFromGLEnum(fFormat); }

    bool hasBaseLevelBeenBoundToFBO() const { return fBaseLevelHasBeenBoundToFBO; }
    void baseLevelWasBoundToFBO() { fBaseLevelHasBeenBoundToFBO = true; }

    static sk_sp<GrGLTexture> MakeWrapped(GrGLGpu*, const GrSurfaceDesc&, GrMipMapsStatus,
                                          const IDDesc&, sk_sp<GrGLTextureParameters>,
                                          GrWrapCacheable, GrIOType);

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, const IDDesc&, sk_sp<GrGLTextureParameters>,
                GrMipMapsStatus);

    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*, const GrSurfaceDesc&, GrMipMapsStatus, const IDDesc&,
                sk_sp<GrGLTextureParameters>, GrWrapCacheable, GrIOType);

    void init(const GrSurfaceDesc&, const IDDesc&);

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override;

private:
    sk_sp<GrGLTextureParameters> fParameters;
    GrGLuint fID;
    GrGLenum fFormat;
    GrBackendObjectOwnership fTextureIDOwnership;
    bool fBaseLevelHasBeenBoundToFBO = false;

    typedef GrTexture INHERITED;
};

#endif
