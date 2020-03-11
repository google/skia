/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/vk/GrVkDescriptorSetManager.h"

#include "src/gpu/vk/GrVkDescriptorPool.h"
#include "src/gpu/vk/GrVkDescriptorSet.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUniformHandler.h"

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateUniformManager(GrVkGpu* gpu) {
    SkSTArray<1, uint32_t> visibilities;
    uint32_t stages = kVertex_GrShaderFlag | kFragment_GrShaderFlag;
    if (gpu->vkCaps().shaderCaps()->geometryShaderSupport()) {
        stages |= kGeometry_GrShaderFlag;
    }
    visibilities.push_back(stages);
    SkTArray<const GrVkSampler*> samplers;
    return Create(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, visibilities, samplers);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateSamplerManager(
        GrVkGpu* gpu, VkDescriptorType type, const GrVkUniformHandler& uniformHandler) {
    SkSTArray<4, uint32_t> visibilities;
    SkSTArray<4, const GrVkSampler*> immutableSamplers;
    SkASSERT(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    for (int i = 0 ; i < uniformHandler.numSamplers(); ++i) {
        visibilities.push_back(uniformHandler.samplerVisibility(i));
        immutableSamplers.push_back(uniformHandler.immutableSampler(i));
    }
    return Create(gpu, type, visibilities, immutableSamplers);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateSamplerManager(
        GrVkGpu* gpu, VkDescriptorType type, const SkTArray<uint32_t>& visibilities) {
    SkSTArray<4, const GrVkSampler*> immutableSamplers;
    SkASSERT(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    for (int i = 0 ; i < visibilities.count(); ++i) {
        immutableSamplers.push_back(nullptr);
    }
    return Create(gpu, type, visibilities, immutableSamplers);
}

VkShaderStageFlags visibility_to_vk_stage_flags(uint32_t visibility) {
    VkShaderStageFlags flags = 0;

    if (visibility & kVertex_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (visibility & kGeometry_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
    }
    if (visibility & kFragment_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    return flags;
}

static bool get_layout_and_desc_count(GrVkGpu* gpu,
                                      VkDescriptorType type,
                                      const SkTArray<uint32_t>& visibilities,
                                      const SkTArray<const GrVkSampler*>& immutableSamplers,
                                      VkDescriptorSetLayout* descSetLayout,
                                      uint32_t* descCountPerSet) {
    if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type ||
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER == type) {
        uint32_t numBindings = visibilities.count();
        std::unique_ptr<VkDescriptorSetLayoutBinding[]> dsSamplerBindings(
                new VkDescriptorSetLayoutBinding[numBindings]);
        for (uint32_t i = 0; i < numBindings; ++i) {
            uint32_t visibility = visibilities[i];
            dsSamplerBindings[i].binding = i;
            dsSamplerBindings[i].descriptorType = type;
            dsSamplerBindings[i].descriptorCount = 1;
            dsSamplerBindings[i].stageFlags = visibility_to_vk_stage_flags(visibility);
            if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type) {
                if (immutableSamplers[i]) {
                    dsSamplerBindings[i].pImmutableSamplers = immutableSamplers[i]->samplerPtr();
                } else {
                    dsSamplerBindings[i].pImmutableSamplers = nullptr;
                }
            }
        }

        VkDescriptorSetLayoutCreateInfo dsSamplerLayoutCreateInfo;
        memset(&dsSamplerLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
        dsSamplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dsSamplerLayoutCreateInfo.pNext = nullptr;
        dsSamplerLayoutCreateInfo.flags = 0;
        dsSamplerLayoutCreateInfo.bindingCount = numBindings;
        // Setting to nullptr fixes an error in the param checker validation layer. Even though
        // bindingCount is 0 (which is valid), it still tries to validate pBindings unless it is
        // null.
        dsSamplerLayoutCreateInfo.pBindings = numBindings ? dsSamplerBindings.get() : nullptr;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skia:8713
        __lsan::ScopedDisabler lsanDisabler;
#endif
        VkResult result;
        GR_VK_CALL_RESULT(gpu, result,
                          CreateDescriptorSetLayout(gpu->device(),
                                                    &dsSamplerLayoutCreateInfo,
                                                    nullptr,
                                                    descSetLayout));
        if (result != VK_SUCCESS) {
            return false;
        }

        *descCountPerSet = visibilities.count();
    } else {
        SkASSERT(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type);
        static constexpr int kUniformDescPerSet = 1;
        SkASSERT(kUniformDescPerSet == visibilities.count());
        // Create Uniform Buffer Descriptor
        VkDescriptorSetLayoutBinding dsUniBinding;
        memset(&dsUniBinding, 0, sizeof(dsUniBinding));
        dsUniBinding.binding = GrVkUniformHandler::kUniformBinding;
        dsUniBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dsUniBinding.descriptorCount = 1;
        dsUniBinding.stageFlags = visibility_to_vk_stage_flags(visibilities[0]);
        dsUniBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo uniformLayoutCreateInfo;
        memset(&uniformLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
        uniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uniformLayoutCreateInfo.pNext = nullptr;
        uniformLayoutCreateInfo.flags = 0;
        uniformLayoutCreateInfo.bindingCount = 1;
        uniformLayoutCreateInfo.pBindings = &dsUniBinding;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skia:8713
        __lsan::ScopedDisabler lsanDisabler;
#endif
        VkResult result;
        GR_VK_CALL_RESULT(gpu, result, CreateDescriptorSetLayout(gpu->device(),
                                                                 &uniformLayoutCreateInfo,
                                                                 nullptr,
                                                                 descSetLayout));
        if (result != VK_SUCCESS) {
            return false;
        }

        *descCountPerSet = kUniformDescPerSet;
    }
    return true;
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::Create(
        GrVkGpu* gpu, VkDescriptorType type,
        const SkTArray<uint32_t>& visibilities,
        const SkTArray<const GrVkSampler*>& immutableSamplers) {
#ifdef SK_DEBUG
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        SkASSERT(visibilities.count() == immutableSamplers.count());
    } else {
        SkASSERT(immutableSamplers.count() == 0);
    }
#endif

    VkDescriptorSetLayout descSetLayout;
    uint32_t descCountPerSet;
    if (!get_layout_and_desc_count(gpu, type, visibilities, immutableSamplers, &descSetLayout,
                                   &descCountPerSet)) {
        return nullptr;
    }
    return new GrVkDescriptorSetManager(gpu, type, descSetLayout, descCountPerSet, visibilities,
                                        immutableSamplers);
}

GrVkDescriptorSetManager::GrVkDescriptorSetManager(
        GrVkGpu* gpu, VkDescriptorType type, VkDescriptorSetLayout descSetLayout,
        uint32_t descCountPerSet, const SkTArray<uint32_t>& visibilities,
        const SkTArray<const GrVkSampler*>& immutableSamplers)
    : fPoolManager(descSetLayout, type, descCountPerSet) {
    for (int i = 0; i < visibilities.count(); ++i) {
        fBindingVisibilities.push_back(visibilities[i]);
    }
    for (int i = 0; i < immutableSamplers.count(); ++i) {
        const GrVkSampler* sampler = immutableSamplers[i];
        if (sampler) {
            sampler->ref();
        }
        fImmutableSamplers.push_back(sampler);
    }
}

const GrVkDescriptorSet* GrVkDescriptorSetManager::getDescriptorSet(GrVkGpu* gpu,
                                                                    const Handle& handle) {
    const GrVkDescriptorSet* ds = nullptr;
    int count = fFreeSets.count();
    if (count > 0) {
        ds = fFreeSets[count - 1];
        fFreeSets.removeShuffle(count - 1);
    } else {
        VkDescriptorSet vkDS;
        if (!fPoolManager.getNewDescriptorSet(gpu, &vkDS)) {
            return nullptr;
        }

        ds = new GrVkDescriptorSet(gpu, vkDS, fPoolManager.fPool, handle);
    }
    SkASSERT(ds);
    return ds;
}

void GrVkDescriptorSetManager::recycleDescriptorSet(const GrVkDescriptorSet* descSet) {
    SkASSERT(descSet);
    fFreeSets.push_back(descSet);
}

void GrVkDescriptorSetManager::release(GrVkGpu* gpu) {
    fPoolManager.freeGPUResources(gpu);

    for (int i = 0; i < fFreeSets.count(); ++i) {
        fFreeSets[i]->unref();
    }
    fFreeSets.reset();

    for (int i = 0; i < fImmutableSamplers.count(); ++i) {
        if (fImmutableSamplers[i]) {
            fImmutableSamplers[i]->unref();
        }
    }
    fImmutableSamplers.reset();
}

bool GrVkDescriptorSetManager::isCompatible(VkDescriptorType type,
                                            const GrVkUniformHandler* uniHandler) const {
    SkASSERT(uniHandler);
    if (type != fPoolManager.fDescType) {
        return false;
    }

    SkASSERT(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    if (fBindingVisibilities.count() != uniHandler->numSamplers()) {
        return false;
    }
    for (int i = 0; i < uniHandler->numSamplers(); ++i) {
        if (uniHandler->samplerVisibility(i) != fBindingVisibilities[i] ||
            uniHandler->immutableSampler(i) != fImmutableSamplers[i]) {
            return false;
        }
    }
    return true;
}

bool GrVkDescriptorSetManager::isCompatible(VkDescriptorType type,
                                            const SkTArray<uint32_t>& visibilities) const {
    if (type != fPoolManager.fDescType) {
        return false;
    }

    if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type ||
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER == type) {
        if (fBindingVisibilities.count() != visibilities.count()) {
            return false;
        }
        for (int i = 0; i < visibilities.count(); ++i) {
            if (visibilities[i] != fBindingVisibilities[i] || fImmutableSamplers[i] != nullptr) {
                return false;
            }
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

GrVkDescriptorSetManager::DescriptorPoolManager::DescriptorPoolManager(
        VkDescriptorSetLayout layout,
        VkDescriptorType type,
        uint32_t descCountPerSet)
    : fDescLayout(layout)
    , fDescType(type)
    , fDescCountPerSet(descCountPerSet)
    , fMaxDescriptors(kStartNumDescriptors)
    , fCurrentDescriptorCount(0)
    , fPool(nullptr) {
}

bool GrVkDescriptorSetManager::DescriptorPoolManager::getNewPool(GrVkGpu* gpu) {
    if (fPool) {
        fPool->unref();
        uint32_t newPoolSize = fMaxDescriptors + ((fMaxDescriptors + 1) >> 1);
        if (newPoolSize < kMaxDescriptors) {
            fMaxDescriptors = newPoolSize;
        } else {
            fMaxDescriptors = kMaxDescriptors;
        }

    }
    fPool = gpu->resourceProvider().findOrCreateCompatibleDescriptorPool(fDescType,
                                                                         fMaxDescriptors);
    return SkToBool(fPool);
}

bool GrVkDescriptorSetManager::DescriptorPoolManager::getNewDescriptorSet(GrVkGpu* gpu,
                                                                          VkDescriptorSet* ds) {
    if (!fMaxDescriptors) {
        return false;
    }
    fCurrentDescriptorCount += fDescCountPerSet;
    if (!fPool || fCurrentDescriptorCount > fMaxDescriptors) {
        if (!this->getNewPool(gpu) ) {
            return false;
        }
        fCurrentDescriptorCount = fDescCountPerSet;
    }

    VkDescriptorSetAllocateInfo dsAllocateInfo;
    memset(&dsAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.pNext = nullptr;
    dsAllocateInfo.descriptorPool = fPool->descPool();
    dsAllocateInfo.descriptorSetCount = 1;
    dsAllocateInfo.pSetLayouts = &fDescLayout;
    VkResult result;
    GR_VK_CALL_RESULT(gpu, result, AllocateDescriptorSets(gpu->device(),
                                                          &dsAllocateInfo,
                                                          ds));
    return result == VK_SUCCESS;
}

void GrVkDescriptorSetManager::DescriptorPoolManager::freeGPUResources(GrVkGpu* gpu) {
    if (fDescLayout) {
        GR_VK_CALL(gpu->vkInterface(), DestroyDescriptorSetLayout(gpu->device(), fDescLayout,
                                                                  nullptr));
        fDescLayout = VK_NULL_HANDLE;
    }

    if (fPool) {
        fPool->unref();
        fPool = nullptr;
    }
}

