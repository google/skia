/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkStencil_DEFINED
#define GrVkStencil_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/vk/GrVkImage.h"

class GrVkImageView;
class GrVkGpu;

class GrVkStencilAttachment : public GrStencilAttachment, public GrVkImage {
public:
    struct Format {
        VkFormat  fInternalFormat;
        int  fStencilBits;
    };

    static GrVkStencilAttachment* Create(GrVkGpu* gpu, SkISize dimensions, int sampleCnt,
                                         const Format& format);

    ~GrVkStencilAttachment() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrManagedResource* imageResource() const { return this->resource(); }
    const GrVkImageView* stencilView() const { return fStencilView.get(); }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrVkStencilAttachment(GrVkGpu* gpu,
                          SkISize dimensions,
                          const Format& format,
                          const GrVkImage::ImageDesc&,
                          const GrVkImageInfo&,
                          sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                          sk_sp<const GrVkImageView> stencilView);

    GrVkGpu* getVkGpu() const;

    sk_sp<const GrVkImageView> fStencilView;
};

#endif
