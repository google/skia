/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkDescriptorSetManager_DEFINED
#define GrVkDescriptorSetManager_DEFINED

#include "GrResourceHandle.h"
#include "GrVkDescriptorPool.h"
#include "SkRefCnt.h"
#include "SkTArray.h"
#include "vk/GrVkDefines.h"

class GrVkDescriptorSet;
class GrVkGpu;

/**
 * This class handles the allocation of descriptor sets for a given VkDescriptorSetLayout. It will
 * try to reuse previously allocated descriptor sets if they are no longer in use by other objects.
 */
class GrVkDescriptorSetManager {
public:
    GR_DEFINE_RESOURCE_HANDLE_CLASS(Handle);

    GrVkDescriptorSetManager(GrVkGpu* gpu,
                             VkDescriptorSetLayout layout,
                             VkDescriptorType,
                             uint32_t samplerCount);
    ~GrVkDescriptorSetManager() {}

    void abandon();
    void release(const GrVkGpu* gpu);

    const GrVkDescriptorSet* getDescriptorSet(GrVkGpu* gpu, const Handle& handle);

    void recycleDescriptorSet(const GrVkDescriptorSet*);

    int isCompatible(uint32_t numSamplers) const { return numSamplers == fNumSamplerBindings; }

private:
    struct DescriptorPoolManager {
        DescriptorPoolManager(VkDescriptorSetLayout layout, VkDescriptorType type,
                              uint32_t samplerCount, GrVkGpu* gpu)
            : fDescLayout(layout)
            , fDescType(type)
            , fCurrentDescriptorCount(0)
            , fPool(nullptr) {
            if (VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type) {
                fDescCountPerSet = kNumUniformDescPerSet;
            } else {
                SkASSERT(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                fDescCountPerSet = samplerCount;
            }

            SkASSERT(fDescCountPerSet < kStartNumDescriptors);
            fMaxDescriptors = kStartNumDescriptors;
            SkASSERT(fMaxDescriptors > 0);
            this->getNewPool(gpu);
        }

        ~DescriptorPoolManager() {
            SkASSERT(!fDescLayout);
            SkASSERT(!fPool);
        }

        void getNewDescriptorSet(GrVkGpu* gpu, VkDescriptorSet* ds);

        void freeGPUResources(const GrVkGpu* gpu);
        void abandonGPUResources();

        VkDescriptorSetLayout  fDescLayout; // Not owned by this class
        VkDescriptorType       fDescType;
        uint32_t               fDescCountPerSet;
        uint32_t               fMaxDescriptors;
        uint32_t               fCurrentDescriptorCount;
        GrVkDescriptorPool*    fPool;

    private:
        enum {
            kNumUniformDescPerSet = 2,
            kMaxDescriptors = 1024,
            kStartNumDescriptors = 16, // must be less than kMaxUniformDescriptors
        };

        void getNewPool(GrVkGpu* gpu);
    };

    DescriptorPoolManager              fPoolManager;
    SkTArray<const GrVkDescriptorSet*> fFreeSets;
    // If the number of bindings is 0 we assume this is for uniform buffers
    uint32_t                           fNumSamplerBindings;
};

#endif
