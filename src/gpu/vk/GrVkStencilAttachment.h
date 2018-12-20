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
#include "vk/GrVkTypes.h"

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

    static GrVkStencilAttachment* Create(GrVkGpu* gpu, int width, int height,
                                         int sampleCnt, const Format& format);

    ~GrVkStencilAttachment() override;

    const GrVkResource* imageResource() const { return this->resource(); }
    const GrVkImageView* stencilView() const { return fStencilView; }

    VkFormat vkFormat() const { return fFormat.fInternalFormat; }

protected:
    void onRelease() override;
    void onAbandon() override;

private:
    size_t onGpuMemorySize() const override;

    GrVkStencilAttachment(GrVkGpu* gpu,
                          const Format& format,
                          const GrVkImage::ImageDesc&,
                          const GrVkImageInfo&,
                          sk_sp<GrVkImageLayout> layout,
                          const GrVkImageView* stencilView);

    GrVkGpu* getVkGpu() const;

    Format fFormat;

    const GrVkImageView*       fStencilView;
};

#endif
