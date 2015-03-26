/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrGLTextureRenderTarget_DEFINED
#define GrGLTextureRenderTarget_DEFINED

#include "GrGLTexture.h"
#include "GrGLRenderTarget.h"

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
                            const GrSurfaceDesc& desc,
                            const GrGLTexture::IDDesc& texIDDesc,
                            const GrGLRenderTarget::IDDesc& rtIDDesc)
        : GrSurface(gpu, texIDDesc.fLifeCycle, desc)
        , GrGLTexture(gpu, desc, texIDDesc, GrGLTexture::kDerived)
        , GrGLRenderTarget(gpu, desc, rtIDDesc, GrGLRenderTarget::kDerived) {
        this->registerWithCache();
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
    // GrGLRenderTarget accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        return GrGLRenderTarget::onGpuMemorySize();
    }

};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
