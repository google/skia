/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTextureRenderTarget_DEFINED
#define GrGLTextureRenderTarget_DEFINED

#include "GrGLGpu.h"
#include "GrGLTexture.h"
#include "GrGLRenderTarget.h"
#include "GrTexturePriv.h"

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
                            const GrGLTexture::IDDesc& texIDDesc,
                            const GrGLRenderTarget::IDDesc& rtIDDesc,
                            bool wasMipMapDataProvided)
        : GrSurface(gpu, desc)
        , GrGLTexture(gpu, desc, texIDDesc, wasMipMapDataProvided)
        , GrGLRenderTarget(gpu, desc, rtIDDesc) {
        this->registerWithCache(budgeted);
    }

    bool canAttemptStencilAttachment() const override;

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

    static sk_sp<GrGLTextureRenderTarget> MakeWrapped(GrGLGpu* gpu, const GrSurfaceDesc& desc,
                                                      const GrGLTexture::IDDesc& texIDDesc,
                                                      const GrGLRenderTarget::IDDesc& rtIDDesc);
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
                            const GrGLTexture::IDDesc& texIDDesc,
                            const GrGLRenderTarget::IDDesc& rtIDDesc,
                            bool wasMipMapDataProvided)
        : GrSurface(gpu, desc)
        , GrGLTexture(gpu, desc, texIDDesc, wasMipMapDataProvided)
        , GrGLRenderTarget(gpu, desc, rtIDDesc) {
        this->registerWithCacheWrapped();
    }

    size_t onGpuMemorySize() const override {
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      this->numSamplesOwnedPerPixel(),
                                      this->texturePriv().hasMipMaps());
    }

};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
