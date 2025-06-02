/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "src/gpu/ganesh/vk/GrVkDescriptorSetManager.h"

#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/vk/GrVkCaps.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorPool.h"
#include "src/gpu/ganesh/vk/GrVkDescriptorSet.h"
#include "src/gpu/ganesh/vk/GrVkGpu.h"
#include "src/gpu/ganesh/vk/GrVkResourceProvider.h"
#include "src/gpu/ganesh/vk/GrVkSampler.h"
#include "src/gpu/ganesh/vk/GrVkUniformHandler.h"
#include "src/gpu/ganesh/vk/GrVkUtil.h"

#include <string.h>
#include <memory>

using namespace skia_private;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
#include <sanitizer/lsan_interface.h>
#endif

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateUniformManager(GrVkGpu* gpu) {
    STArray<1, uint32_t> visibilities;
    uint32_t stages = kVertex_GrShaderFlag | kFragment_GrShaderFlag;
    visibilities.push_back(stages);
    TArray<const GrVkSampler*> samplers;
    return Create(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, visibilities, samplers);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateSamplerManager(
        GrVkGpu* gpu, VkDescriptorType type, const GrVkUniformHandler& uniformHandler) {
    STArray<4, uint32_t> visibilities;
    STArray<4, const GrVkSampler*> immutableSamplers;
    SkASSERT(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    for (int i = 0 ; i < uniformHandler.numSamplers(); ++i) {
        visibilities.push_back(uniformHandler.samplerVisibility(i));
        immutableSamplers.push_back(uniformHandler.immutableSampler(i));
    }
    return Create(gpu, type, visibilities, immutableSamplers);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateZeroSamplerManager(GrVkGpu* gpu) {
    TArray<uint32_t> visibilities;
    TArray<const GrVkSampler*> immutableSamplers;
    return Create(gpu, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, visibilities, immutableSamplers);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateInputManager(GrVkGpu* gpu) {
    STArray<1, uint32_t> visibilities;
    visibilities.push_back(kFragment_GrShaderFlag);
    TArray<const GrVkSampler*> samplers;
    return Create(gpu, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, visibilities, samplers);
}

VkShaderStageFlags visibility_to_vk_stage_flags(uint32_t visibility) {
    VkShaderStageFlags flags = 0;

    if (visibility & kVertex_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_VERTEX_BIT;
    }
    if (visibility & kFragment_GrShaderFlag) {
        flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    return flags;
}

static bool get_layout_and_desc_count(GrVkGpu* gpu,
                                      VkDescriptorType type,
                                      const TArray<uint32_t>& visibilities,
                                      const TArray<const GrVkSampler*>& immutableSamplers,
                                      VkDescriptorSetLayout* descSetLayout,
                                      uint32_t* descCountPerSet) {
    if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type ||
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER == type) {
        uint32_t numBindings = visibilities.size();
        std::unique_ptr<VkDescriptorSetLayoutBinding[]> dsSamplerBindings(
                new VkDescriptorSetLayoutBinding[numBindings]);
        *descCountPerSet = 0;
        for (uint32_t i = 0; i < numBindings; ++i) {
            uint32_t visibility = visibilities[i];
            dsSamplerBindings[i].binding = i;
            dsSamplerBindings[i].descriptorType = type;
            dsSamplerBindings[i].descriptorCount = 1;
            dsSamplerBindings[i].stageFlags = visibility_to_vk_stage_flags(visibility);
            if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type) {
                if (immutableSamplers[i]) {
                    (*descCountPerSet) += gpu->vkCaps().ycbcrCombinedImageSamplerDescriptorCount();
                    dsSamplerBindings[i].pImmutableSamplers = immutableSamplers[i]->samplerPtr();
                } else {
                    (*descCountPerSet)++;
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
        // skbug.com/40040004
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
    } else if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
        static constexpr int kUniformDescPerSet = 1;
        SkASSERT(kUniformDescPerSet == visibilities.size());
        // Create Uniform Buffer Descriptor
        VkDescriptorSetLayoutBinding dsUniBinding;
        dsUniBinding.binding = GrVkUniformHandler::kUniformBinding;
        dsUniBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dsUniBinding.descriptorCount = 1;
        dsUniBinding.stageFlags = visibility_to_vk_stage_flags(visibilities[0]);
        dsUniBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo uniformLayoutCreateInfo;
        uniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uniformLayoutCreateInfo.pNext = nullptr;
        uniformLayoutCreateInfo.flags = 0;
        uniformLayoutCreateInfo.bindingCount = 1;
        uniformLayoutCreateInfo.pBindings = &dsUniBinding;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skbug.com/40040004
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
    } else {
        SkASSERT(type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);
        static constexpr int kInputDescPerSet = 1;
        SkASSERT(kInputDescPerSet == visibilities.size());

        // Create Input Buffer Descriptor
        VkDescriptorSetLayoutBinding dsInpuBinding;
        dsInpuBinding.binding = 0;
        dsInpuBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        dsInpuBinding.descriptorCount = 1;
        SkASSERT(visibilities[0] == kFragment_GrShaderFlag);
        dsInpuBinding.stageFlags = visibility_to_vk_stage_flags(visibilities[0]);
        dsInpuBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo inputLayoutCreateInfo;
        inputLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        inputLayoutCreateInfo.pNext = nullptr;
        inputLayoutCreateInfo.flags = 0;
        inputLayoutCreateInfo.bindingCount = 1;
        inputLayoutCreateInfo.pBindings = &dsInpuBinding;

#if defined(SK_ENABLE_SCOPED_LSAN_SUPPRESSIONS)
        // skbug.com/40040004
        __lsan::ScopedDisabler lsanDisabler;
#endif
        VkResult result;
        GR_VK_CALL_RESULT(gpu, result, CreateDescriptorSetLayout(gpu->device(),
                                                                 &inputLayoutCreateInfo,
                                                                 nullptr, descSetLayout));
        if (result != VK_SUCCESS) {
            return false;
        }

        *descCountPerSet = kInputDescPerSet;
    }
    return true;
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::Create(
        GrVkGpu* gpu, VkDescriptorType type,
        const TArray<uint32_t>& visibilities,
        const TArray<const GrVkSampler*>& immutableSamplers) {
#ifdef SK_DEBUG
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        SkASSERT(visibilities.size() == immutableSamplers.size());
    } else {
        SkASSERT(immutableSamplers.empty());
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
        uint32_t descCountPerSet, const TArray<uint32_t>& visibilities,
        const TArray<const GrVkSampler*>& immutableSamplers)
    : fPoolManager(descSetLayout, type, descCountPerSet) {
    for (int i = 0; i < visibilities.size(); ++i) {
        fBindingVisibilities.push_back(visibilities[i]);
    }
    for (int i = 0; i < immutableSamplers.size(); ++i) {
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
    int count = fFreeSets.size();
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

    for (int i = 0; i < fFreeSets.size(); ++i) {
        fFreeSets[i]->unref();
    }
    fFreeSets.clear();

    for (int i = 0; i < fImmutableSamplers.size(); ++i) {
        if (fImmutableSamplers[i]) {
            fImmutableSamplers[i]->unref();
        }
    }
    fImmutableSamplers.clear();
}

bool GrVkDescriptorSetManager::isCompatible(VkDescriptorType type,
                                            const GrVkUniformHandler* uniHandler) const {
    SkASSERT(uniHandler);
    if (type != fPoolManager.fDescType) {
        return false;
    }

    SkASSERT(type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    if (fBindingVisibilities.size() != uniHandler->numSamplers()) {
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

bool GrVkDescriptorSetManager::isZeroSampler() const {
    if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER != fPoolManager.fDescType) {
        return false;
    }
    if (!fBindingVisibilities.empty()) {
        return false;
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

