/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrMtlStencil_DEFINED
#define GrMtlStencil_DEFINED

#include "src/gpu/GrStencilAttachment.h"

#import <Metal/Metal.h>

class GrMtlImageView;
class GrMtlGpu;

class GrMtlStencilAttachment : public GrStencilAttachment {
public:
    struct Format {
        MTLPixelFormat fInternalFormat;
        int  fStencilBits;
        int  fTotalBits;
        bool fPacked;
    };

    static GrMtlStencilAttachment* Create(GrMtlGpu* gpu, SkISize dimensions,
                                          int sampleCnt, const Format& format);

    ~GrMtlStencilAttachment() override;

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMtl(fStencilView.pixelFormat);
    }

    MTLPixelFormat mtlFormat() const { return fFormat.fInternalFormat; }

    id<MTLTexture> stencilView() const { return fStencilView; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrMtlStencilAttachment(GrMtlGpu* gpu,
                           SkISize dimensions,
                           const Format& format,
                           const id<MTLTexture> stencilView);

    GrMtlGpu* getMtlGpu() const;

    Format fFormat;

    id<MTLTexture> fStencilView;
};

#endif
