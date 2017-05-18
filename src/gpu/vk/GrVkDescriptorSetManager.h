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
class GrVkUniformHandler;

/**
 * This class handles the allocation of descriptor sets for a given VkDescriptorSetLayout. It will
 * try to reuse previously allocated descriptor sets if they are no longer in use by other objects.
 */
class GrVkDescriptorSetManager {
public:
    GR_DEFINE_RESOURCE_HANDLE_CLASS(Handle);

    static GrVkDescriptorSetManager* CreateUniformManager(GrVkGpu* gpu);
    static GrVkDescriptorSetManager* CreateSamplerManager(GrVkGpu* gpu, VkDescriptorType type,
                                                          const GrVkUniformHandler&);
    static GrVkDescriptorSetManager* CreateSamplerManager(GrVkGpu* gpu, VkDescriptorType type,
                                                          const SkTArray<uint32_t>& visibilities);

    ~GrVkDescriptorSetManager() {}

    void abandon();
    void release(const GrVkGpu* gpu);

    VkDescriptorSetLayout layout() const { return fPoolManager.fDescLayout; }

    const GrVkDescriptorSet* getDescriptorSet(GrVkGpu* gpu, const Handle& handle);

    void recycleDescriptorSet(const GrVkDescriptorSet*);

    bool isCompatible(VkDescriptorType type, const GrVkUniformHandler*) const;
    bool isCompatible(VkDescriptorType type,
                      const SkTArray<uint32_t>& visibilities) const;

private:
    struct DescriptorPoolManager {
        DescriptorPoolManager(VkDescriptorType type, GrVkGpu* gpu,
                              const SkTArray<uint32_t>& visibilities);


        ~DescriptorPoolManager() {
            SkASSERT(!fDescLayout);
            SkASSERT(!fPool);
        }

        void getNewDescriptorSet(GrVkGpu* gpu, VkDescriptorSet* ds);

        void freeGPUResources(const GrVkGpu* gpu);
        void abandonGPUResources();

        VkDescriptorSetLayout  fDescLayout;
        VkDescriptorType       fDescType;
        uint32_t               fDescCountPerSet;
        uint32_t               fMaxDescriptors;
        uint32_t               fCurrentDescriptorCount;
        GrVkDescriptorPool*    fPool;

    private:
        enum {
            kUniformDescPerSet = 2,
            kMaxDescriptors = 1024,
            kStartNumDescriptors = 16, // must be less than kMaxUniformDescriptors
        };

        void getNewPool(GrVkGpu* gpu);
    };

    GrVkDescriptorSetManager(GrVkGpu* gpu,
                             VkDescriptorType,
                             const SkTArray<uint32_t>& visibilities);


    DescriptorPoolManager                    fPoolManager;
    SkTArray<const GrVkDescriptorSet*, true> fFreeSets;
    SkSTArray<4, uint32_t>                   fBindingVisibilities;
};

#endif
