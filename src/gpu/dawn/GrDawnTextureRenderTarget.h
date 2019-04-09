/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnTextureRenderTarget_DEFINED
#define GrDawnTextureRenderTarget_DEFINED

#include "GrDawnTexture.h"
#include "GrDawnRenderTarget.h"

class GrDawnGpu;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrDawnTextureRenderTarget : public GrDawnTexture, public GrDawnRenderTarget {
public:
    // We're virtually derived from GrSurface (via both GrDawnTexture and GrDawnRenderTarget) so its
    // constructor must be explicitly called.
    GrDawnTextureRenderTarget(GrDawnGpu* gpu,
                             const dawn::Texture texture,
                             const dawn::TextureView textureView,
                             const GrSurfaceDesc& desc,
                             const GrDawnImageInfo& info,
                             GrMipMapsStatus mipMapsStatus);

    bool canAttemptStencilAttachment() const override;

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

    GrBackendFormat backendFormat() const override { return GrDawnTexture::backendFormat(); }

protected:
    void onAbandon() override {
        GrDawnRenderTarget::onAbandon();
        GrDawnTexture::onAbandon();
    }

    void onRelease() override {
        GrDawnRenderTarget::onRelease();
        GrDawnTexture::onRelease();
    }

private:
    size_t onGpuMemorySize() const override;
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
