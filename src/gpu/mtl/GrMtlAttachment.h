/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlStencil_DEFINED
#define GrMtlStencil_DEFINED

#include "src/gpu/mtl/GrMtlAttachment.h"

#import <Metal/Metal.h>

class GrMtlImageView;
class GrMtlGpu;

class GrMtlAttachment : public GrAttachment {
public:
    static GrMtlAttachment* Create(GrMtlGpu* gpu,
                                   SkISize dimensions,
                                   int sampleCnt,
                                   MTLPixelFormat format);

    ~GrMtlAttachment() override;

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMtl(fStencilView.pixelFormat);
    }

    MTLPixelFormat mtlFormat() const { return fStencilView.pixelFormat; }

    id<MTLTexture> stencilView() const { return fStencilView; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrMtlAttachment(GrMtlGpu* gpu, SkISize dimensions, const id<MTLTexture> stencilView);

    GrMtlGpu* getMtlGpu() const;

    id<MTLTexture> fStencilView;
};

#endif
