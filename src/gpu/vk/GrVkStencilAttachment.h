/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkStencil_DEFINED
#define GrVkStencil_DEFINED

#include "GrStencilAttachment.h"
#include "GrVkImage.h"
#include "vk/GrVkDefines.h"

class GrVkImageView;
class GrVkGpu;

class GrVkStencilAttachment : public GrStencilAttachment, public GrVkImage {
public:
    struct Format {
        VkFormat  fInternalFormat;
        int  fStencilBits;
        int  fTotalBits;
        bool fPacked;
    };

    static GrVkStencilAttachment* Create(GrVkGpu* gpu, GrGpuResource::LifeCycle lifeCycle,
                                         int width, int height,
                                         int sampleCnt, const Format& format);

    ~GrVkStencilAttachment() override;

    const GrVkImage::Resource* imageResource() const { return this->resource(); }
    const GrVkImageView* stencilView() const { return fStencilView; }

    VkFormat vkFormat() const { return fFormat.fInternalFormat; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrVkStencilAttachment(GrVkGpu* gpu,
                          GrGpuResource::LifeCycle lifeCycle,
                          const Format& format,
                          const GrVkImage::ImageDesc&,
                          const GrVkImage::Resource*,
                          const GrVkImageView* stencilView);

    GrVkGpu* getVkGpu() const;

    Format fFormat;

    const GrVkImageView*       fStencilView;
};

#endif
