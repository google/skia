/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkAttachment_DEFINED
#define GrVkAttachment_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrAttachment.h"
#include "src/gpu/vk/GrVkImage.h"

class GrVkImageView;
class GrVkGpu;

class GrVkAttachment : public GrAttachment, public GrVkImage {
public:
    static sk_sp<GrVkAttachment> MakeStencil(GrVkGpu* gpu,
                                             SkISize dimensions,
                                             int sampleCnt,
                                             VkFormat format);

    ~GrVkAttachment() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrManagedResource* imageResource() const { return this->resource(); }
    const GrVkImageView* stencilView() const { return fStencilView.get(); }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrVkAttachment(GrVkGpu* gpu,
                   SkISize dimensions,
                   UsageFlags supportedUsages,
                   VkFormat format,
                   const GrVkImage::ImageDesc&,
                   const GrVkImageInfo&,
                   sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                   sk_sp<const GrVkImageView> stencilView);

    GrVkGpu* getVkGpu() const;

    sk_sp<const GrVkImageView> fStencilView;
};

#endif
