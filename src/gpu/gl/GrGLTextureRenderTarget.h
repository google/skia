/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTextureRenderTarget_DEFINED
#define GrGLTextureRenderTarget_DEFINED

#include "src/gpu/gl/GrGLRenderTarget.h"
#include "src/gpu/gl/GrGLTexture.h"

class GrGLGpu;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrGLTextureRenderTarget : public GrGLTexture, public GrGLRenderTarget {
public:
    // We're virtually derived from GrSurface (via both GrGLTexture and GrGLRenderTarget) so its
    // constructor must be explicitly called.
    GrGLTextureRenderTarget(GrGLGpu* gpu,
                            SkBudgeted budgeted,
                            const GrSurfaceDesc& desc,
                            int sampleCount,
                            const GrGLTexture::IDDesc& texIDDesc,
                            const GrGLRenderTarget::IDDesc& rtIDDesc,
                            GrMipMapsStatus);

    bool canAttemptStencilAttachment() const override;

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

    static sk_sp<GrGLTextureRenderTarget> MakeWrapped(GrGLGpu* gpu, const GrSurfaceDesc& desc,
                                                      int sampleCount,
                                                      const GrGLTexture::IDDesc& texIDDesc,
                                                      sk_sp<GrGLTextureParameters> parameters,
                                                      const GrGLRenderTarget::IDDesc& rtIDDesc,
                                                      GrWrapCacheable cacheble, GrMipMapsStatus);

    GrBackendFormat backendFormat() const override {
        // It doesn't matter if we take the texture or render target path, so just pick texture.
        return GrGLTexture::backendFormat();
    }

protected:
    void onAbandon() override {
        GrGLRenderTarget::onAbandon();
        GrGLTexture::onAbandon();
    }

    void onRelease() override {
        GrGLRenderTarget::onRelease();
        GrGLTexture::onRelease();
    }

private:
    // Constructor for instances wrapping backend objects.
    GrGLTextureRenderTarget(GrGLGpu* gpu,
                            const GrSurfaceDesc& desc,
                            int sampleCount,
                            const GrGLTexture::IDDesc& texIDDesc,
                            sk_sp<GrGLTextureParameters> parameters,
                            const GrGLRenderTarget::IDDesc& rtIDDesc,
                            GrWrapCacheable,
                            GrMipMapsStatus);

    size_t onGpuMemorySize() const override;
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
