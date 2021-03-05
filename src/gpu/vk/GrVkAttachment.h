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

    static sk_sp<GrVkAttachment> MakeTexture(GrVkGpu* gpu,
                                             SkISize dimensions,
                                             VkFormat format,
                                             uint32_t mipLevels,
                                             GrRenderable renderable,
                                             int numSamples,
                                             SkBudgeted budgeted,
                                             GrProtected isProtected);

    static sk_sp<GrVkAttachment> MakeWrapped(GrVkGpu* gpu,
                                             SkISize dimensions,
                                             const GrVkImageInfo&,
                                             sk_sp<GrBackendSurfaceMutableStateImpl>,
                                             UsageFlags attachmentUsages,
                                             GrWrapOwnership,
                                             GrWrapCacheable,
                                             bool forSecondaryCB = false);

    ~GrVkAttachment() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrManagedResource* imageResource() const { return this->resource(); }
    const GrVkImageView* framebufferView() const { return fFramebufferView.get(); }
    const GrVkImageView* textureView() const { return fTextureView.get(); }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    static sk_sp<GrVkAttachment> Make(GrVkGpu* gpu,
                                      SkISize dimensions,
                                      UsageFlags attachmentUsages,
                                      int sampleCnt,
                                      VkFormat format,
                                      uint32_t mipLevels,
                                      VkImageUsageFlags vkUsageFlags,
                                      GrProtected isProtected,
                                      SkBudgeted);

    GrVkAttachment(GrVkGpu* gpu,
                   SkISize dimensions,
                   UsageFlags supportedUsages,
                   const GrVkImageInfo&,
                   sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                   sk_sp<const GrVkImageView> framebufferView,
                   sk_sp<const GrVkImageView> textureView,
                   SkBudgeted);

    GrVkAttachment(GrVkGpu* gpu,
                   SkISize dimensions,
                   UsageFlags supportedUsages,
                   const GrVkImageInfo&,
                   sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                   sk_sp<const GrVkImageView> framebufferView,
                   sk_sp<const GrVkImageView> textureView,
                   GrBackendObjectOwnership,
                   GrWrapCacheable,
                   bool forSecondaryCB);

    GrVkGpu* getVkGpu() const;

    sk_sp<const GrVkImageView> fFramebufferView;
    sk_sp<const GrVkImageView> fTextureView;
};

#endif
