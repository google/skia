/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrGLTextureRenderTarget_DEFINED
#define GrGLTextureRenderTarget_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "src/gpu/ganesh/gl/GrGLRenderTarget.h"
#include "src/gpu/ganesh/gl/GrGLTexture.h"

#include <cstddef>
#include <string_view>

class GrGLGpu;
class GrGLTextureParameters;
class SkTraceMemoryDump;
enum class GrMipmapStatus;
enum class GrWrapCacheable : bool;
namespace skgpu {
enum class Budgeted : bool;
}

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
                            skgpu::Budgeted budgeted,
                            int sampleCount,
                            const GrGLTexture::Desc& texDesc,
                            const GrGLRenderTarget::IDs&,
                            GrMipmapStatus,
                            std::string_view label);

    bool canAttemptStencilAttachment(bool useMultisampleFBO) const override;

    void dumpMemoryStatistics(SkTraceMemoryDump* traceMemoryDump) const override;

    static sk_sp<GrGLTextureRenderTarget> MakeWrapped(GrGLGpu* gpu,
                                                      int sampleCount,
                                                      const GrGLTexture::Desc&,
                                                      sk_sp<GrGLTextureParameters>,
                                                      const GrGLRenderTarget::IDs&,
                                                      GrWrapCacheable,
                                                      GrMipmapStatus,
                                                      std::string_view label);

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
                            int sampleCount,
                            const GrGLTexture::Desc& texDesc,
                            sk_sp<GrGLTextureParameters> parameters,
                            const GrGLRenderTarget::IDs& ids,
                            GrWrapCacheable,
                            GrMipmapStatus,
                            std::string_view label);

    size_t onGpuMemorySize() const override;

    void onSetLabel() override;
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
