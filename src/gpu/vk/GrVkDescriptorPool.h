/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkDescriptorPool_DEFINED
#define GrVkDescriptorPool_DEFINED

#include "GrVkResource.h"

#include "vk/GrVkDefines.h"

class GrVkGpu;

/**
 * We require that all descriptor sets are of a single descriptor type. We also use a pool to only
 * make one type of descriptor set. Thus a single VkDescriptorPool will only allocated space for
 * for one type of descriptor.
 */
class GrVkDescriptorPool : public GrVkResource {
public:
    explicit GrVkDescriptorPool(const GrVkGpu* gpu, VkDescriptorType type, uint32_t count);

    VkDescriptorPool descPool() const { return fDescPool; }

    void reset(const GrVkGpu* gpu);

    // Returns whether or not this descriptor pool could be used, assuming it gets fully reset and
    // not in use by another draw, to support the requested type and count.
    bool isCompatible(VkDescriptorType type, uint32_t count) const;

private:
    void freeGPUData(const GrVkGpu* gpu) const override;

    VkDescriptorType     fType;
    uint32_t             fCount;
    VkDescriptorPool     fDescPool;

    typedef GrVkResource INHERITED;
};

#endif
