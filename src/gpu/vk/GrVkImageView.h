/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkImageView_DEFINED
#define GrVkImageView_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkManagedResource.h"

class GrVkSamplerYcbcrConversion;
struct GrVkYcbcrConversionInfo;

class GrVkImageView : public GrVkManagedResource {
public:
    enum Type {
        kColor_Type,
        kStencil_Type
    };

    static sk_sp<const GrVkImageView> Make(GrVkGpu* gpu, VkImage image, VkFormat format,
                                           Type viewType, uint32_t miplevels,
                                           const GrVkYcbcrConversionInfo& ycbcrInfo);

    VkImageView imageView() const { return fImageView; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkImageView: %d (%d refs)\n", fImageView, this->getRefCnt());
    }
#endif

private:
    GrVkImageView(const GrVkGpu* gpu, VkImageView imageView,
                  GrVkSamplerYcbcrConversion* ycbcrConversion)
            : INHERITED(gpu), fImageView(imageView), fYcbcrConversion(ycbcrConversion) {}

    void freeGPUData() const override;

    VkImageView  fImageView;
    GrVkSamplerYcbcrConversion* fYcbcrConversion;

    using INHERITED = GrVkManagedResource;
};

#endif
