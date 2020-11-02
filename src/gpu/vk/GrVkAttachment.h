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

    static sk_sp<GrVkAttachment> MakeMSAA(GrVkGpu* gpu,
                                          SkISize dimensions,
                                          int numSamples,
                                          VkFormat format,
                                          GrProtected isProtected);

    static sk_sp<GrAttachment> MakeWrapped(GrVkGpu* gpu,
                                           SkISize dimensions,
                                           const GrVkImageInfo&,
                                           sk_sp<GrBackendSurfaceMutableStateImpl>,
                                           UsageFlags attachmentUsages,
                                           GrWrapOwnership,
                                           GrWrapCacheable);

    ~GrVkAttachment() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrManagedResource* imageResource() const { return this->resource(); }
    const GrVkImageView* view() const { return fView.get(); }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    static sk_sp<GrVkAttachment> Make(GrVkGpu* gpu,
                                      SkISize dimensions,
                                      UsageFlags attachmentUsages,
                                      int sampleCnt,
                                      VkFormat format,
                                      VkImageUsageFlags vkUsageFlags,
                                      GrProtected isProtected,
                                      SkBudgeted);

    GrVkAttachment(GrVkGpu* gpu,
                   SkISize dimensions,
                   UsageFlags supportedUsages,
                   const GrVkImageInfo&,
                   sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                   sk_sp<const GrVkImageView> view,
                   SkBudgeted);

    GrVkAttachment(GrVkGpu* gpu,
                   SkISize dimensions,
                   UsageFlags supportedUsages,
                   const GrVkImageInfo&,
                   sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                   sk_sp<const GrVkImageView> view,
                   GrBackendObjectOwnership,
                   GrWrapCacheable);

    GrVkGpu* getVkGpu() const;

    sk_sp<const GrVkImageView> fView;
};

#endif
