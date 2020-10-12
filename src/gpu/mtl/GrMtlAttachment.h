/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlAttachment_DEFINED
#define GrMtlAttachment_DEFINED

#include "src/gpu/GrAttachment.h"

#import <Metal/Metal.h>

class GrMtlImageView;
class GrMtlGpu;

class GrMtlAttachment : public GrAttachment {
public:
    static sk_sp<GrMtlAttachment> MakeStencil(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              int sampleCnt,
                                              MTLPixelFormat format);

    ~GrMtlAttachment() override;

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMtl(fView.pixelFormat);
    }

    MTLPixelFormat mtlFormat() const { return fView.pixelFormat; }

    id<MTLTexture> view() const { return fView; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    const id<MTLTexture> View);

    GrMtlGpu* getMtlGpu() const;

    id<MTLTexture> fView;
};

#endif
