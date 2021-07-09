/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlAttachment_DEFINED
#define GrMtlAttachment_DEFINED

#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/GrAttachment.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlAttachment : public GrAttachment {
public:
    static sk_sp<GrMtlAttachment> MakeStencil(GrMtlGpu* gpu,
                                              SkISize dimensions,
                                              int sampleCnt,
                                              MTLPixelFormat format);

    ~GrMtlAttachment() override;

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMtl(fTexture.pixelFormat);
    }

    MTLPixelFormat mtlFormat() const { return fTexture.pixelFormat; }

    id<MTLTexture> mtlTexture() const { return fTexture; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    static sk_sp<GrMtlAttachment> Make(GrMtlGpu* gpu,
                                       SkISize dimensions,
                                       UsageFlags attachmentUsages,
                                       int sampleCnt,
                                       MTLPixelFormat format,
                                       uint32_t mipLevels,
                                       int mtlTextureUsage,
                                       int mtlStorageMode,
                                       GrProtected isProtected,
                                       SkBudgeted);
    GrMtlAttachment(GrMtlGpu* gpu,
                    SkISize dimensions,
                    UsageFlags supportedUsages,
                    id<MTLTexture> texture,
                    SkBudgeted);

    GrMtlGpu* getMtlGpu() const;

    id<MTLTexture> fTexture;
};

#endif
