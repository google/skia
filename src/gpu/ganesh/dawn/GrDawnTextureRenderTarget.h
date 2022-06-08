/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnTextureRenderTarget_DEFINED
#define GrDawnTextureRenderTarget_DEFINED

#include "src/gpu/ganesh/dawn/GrDawnRenderTarget.h"
#include "src/gpu/ganesh/dawn/GrDawnTexture.h"

class GrDawnGpu;

#ifdef SK_BUILD_FOR_WIN
// Windows gives bogus warnings about inheriting asTexture/asRenderTarget via dominance.
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

class GrDawnTextureRenderTarget : public GrDawnTexture, public GrDawnRenderTarget {
public:
    GrDawnTextureRenderTarget(GrDawnGpu* gpu,
                              SkISize dimensions,
                              int sampleCnt,
                              const GrDawnTextureInfo& textureInfo,
                              GrMipmapStatus mipmapStatus,
                              std::string_view label);

    bool canAttemptStencilAttachment(bool useMSAASurface) const override;

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

    void onSetLabel() override{}
};

#ifdef SK_BUILD_FOR_WIN
#pragma warning(pop)
#endif

#endif
