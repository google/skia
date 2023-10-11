/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTexture_DEFINED
#define GrGLTexture_DEFINED

#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/gl/GrGLTypesPriv.h"
#include "src/gpu/ganesh/gl/GrGLUtil.h"

class GrGLGpu;

class GrGLTexture : public GrTexture {
public:
    struct Desc {
        SkISize fSize                       = {-1, -1};
        GrGLenum fTarget                    = 0;
        GrGLuint fID                        = 0;
        GrGLFormat fFormat                  = GrGLFormat::kUnknown;
        GrBackendObjectOwnership fOwnership = GrBackendObjectOwnership::kOwned;
        skgpu::Protected fIsProtected       = skgpu::Protected::kNo;
    };

    static GrTextureType TextureTypeFromTarget(GrGLenum textureTarget);

    GrGLTexture(GrGLGpu*, skgpu::Budgeted, const Desc&, GrMipmapStatus, std::string_view label);

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
                                          GrMipmapStatus,
                                          const Desc&,
                                          sk_sp<GrGLTextureParameters>,
                                          GrWrapCacheable, GrIOType,
                                          std::string_view label);

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    // Constructor for subclasses.
    GrGLTexture(GrGLGpu*,
                const Desc&,
                sk_sp<GrGLTextureParameters>,
                GrMipmapStatus,
                std::string_view label);

    // Constructor for instances wrapping backend objects.
    GrGLTexture(GrGLGpu*,
                const Desc&,
                GrMipmapStatus,
                sk_sp<GrGLTextureParameters>,
                GrWrapCacheable,
                GrIOType,
                std::string_view label);

    void init(const Desc&);

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImages::BackendTextureReleaseProc*) override;

    void onSetLabel() override;

private:
    sk_sp<GrGLTextureParameters> fParameters;
    GrGLuint fID;
    GrGLFormat fFormat;
    GrBackendObjectOwnership fTextureIDOwnership;
    bool fBaseLevelHasBeenBoundToFBO = false;

    using INHERITED = GrTexture;
};

#endif
