/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTTextureRenderTarget_DEFINED
#define GrNXTTextureRenderTarget_DEFINED

#include "GrNXTTexture.h"
#include "GrNXTRenderTarget.h"

class GrNXTGpu;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrNXTTextureRenderTarget : public GrNXTTexture, public GrNXTRenderTarget {
public:
    // We're virtually derived from GrSurface (via both GrNXTTexture and GrNXTRenderTarget) so its
    // constructor must be explicitly called.
    GrNXTTextureRenderTarget(GrNXTGpu* gpu,
                             const dawn::Texture texture,
                             const dawn::TextureView textureView,
                             const GrSurfaceDesc& desc,
                             const GrNXTImageInfo& info,
                             GrMipMapsStatus mipMapsStatus);

    bool canAttemptStencilAttachment() const override;

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

protected:
    void onAbandon() override {
        GrNXTRenderTarget::onAbandon();
        GrNXTTexture::onAbandon();
    }

    void onRelease() override {
        GrNXTRenderTarget::onRelease();
        GrNXTTexture::onRelease();
    }

private:
    size_t onGpuMemorySize() const override;
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
