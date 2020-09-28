/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrVkMSAAAttachment_DEFINED
#define GrVkMSAAAttachment_DEFINED

#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/GrMSAAAttachment.h"
#include "src/gpu/vk/GrVkImage.h"

class GrVkImageView;

class GrVkMSAAAttachment : public GrMSAAAttachment, public GrVkImage {
public:
    static sk_sp<GrVkMSAAAttachment> Make(GrVkGpu* gpu,
                                          SkISize dimensions,
                                          const GrVkImage::ImageDesc& imageDesc);

    ~GrVkMSAAAttachment() override;

    GrBackendFormat backendFormat() const override { return this->getBackendFormat(); }

    const GrManagedResource* imageResource() const { return this->resource(); }
    const GrVkImageView* view() const { return fView.get(); }

private:
    void onRelease() override;
    void onAbandon() override;

    size_t onGpuMemorySize() const override;

    GrVkMSAAAttachment(GrVkGpu* gpu,
                       SkISize dimensions,
                       int sampleCnt,
                       const GrVkImageInfo& info,
                       sk_sp<GrBackendSurfaceMutableStateImpl> mutableState,
                       sk_sp<const GrVkImageView> view);

    sk_sp<const GrVkImageView> fView;
};

#endif


