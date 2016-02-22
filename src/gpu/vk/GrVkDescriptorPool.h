/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkDescriptorPool_DEFINED
#define GrVkDescriptorPool_DEFINED

#include "GrVkResource.h"

#include "vulkan/vulkan.h"

class GrVkGpu;

class GrVkDescriptorPool : public GrVkResource {
public:
    class DescriptorTypeCounts {
    public:
        DescriptorTypeCounts() {
            memset(fDescriptorTypeCount, 0, sizeof(fDescriptorTypeCount));
        }

        void setTypeCount(VkDescriptorType type, uint8_t count);
        int numPoolSizes() const;

        // Determines if for each i, that.fDescriptorTypeCount[i] <= fDescriptorTypeCount[i];
        bool isSuperSet(const DescriptorTypeCounts& that) const;
    private:
        uint8_t fDescriptorTypeCount[VK_DESCRIPTOR_TYPE_RANGE_SIZE];

        friend class GrVkDescriptorPool;
    };

    explicit GrVkDescriptorPool(const GrVkGpu* gpu, const DescriptorTypeCounts& typeCounts);

    VkDescriptorPool descPool() const { return fDescPool; }

    void reset(const GrVkGpu* gpu);

    // Returns whether or not this descriptor pool could be used, assuming it gets fully reset and
    // not in use by another draw, to support the requested typeCounts.
    bool isCompatible(const DescriptorTypeCounts& typeCounts) const;

private:
    void freeGPUData(const GrVkGpu* gpu) const override;

    DescriptorTypeCounts fTypeCounts;
    VkDescriptorPool     fDescPool;

    typedef GrVkResource INHERITED;
};

#endif
