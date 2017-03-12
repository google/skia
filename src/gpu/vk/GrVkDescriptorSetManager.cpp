/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "GrVkDescriptorSetManager.h"

#include "GrVkDescriptorPool.h"
#include "GrVkDescriptorSet.h"
#include "GrVkGpu.h"
#include "GrVkUniformHandler.h"

GrVkDescriptorSetManager::GrVkDescriptorSetManager(GrVkGpu* gpu,
                                                   VkDescriptorType type,
                                                   const GrVkUniformHandler* uniformHandler)
    : fPoolManager(type, gpu, uniformHandler) {
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        SkASSERT(uniformHandler);
        for (int i = 0; i < uniformHandler->numSamplers(); ++i) {
            fBindingVisibilities.push_back(uniformHandler->samplerVisibility(i));
        }
    } else {
        SkASSERT(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        // We set the visibility of the first binding to the vertex shader and the second to the
        // fragment shader.
        fBindingVisibilities.push_back(kVertex_GrShaderFlag);
        fBindingVisibilities.push_back(kFragment_GrShaderFlag);
    }
}

GrVkDescriptorSetManager::GrVkDescriptorSetManager(GrVkGpu* gpu,
                                                   VkDescriptorType type,
                                                   const SkTArray<uint32_t>& visibilities)
    : fPoolManager(type, gpu, visibilities) {
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        for (int i = 0; i < visibilities.count(); ++i) {
            fBindingVisibilities.push_back(visibilities[i]);
        }
    } else {
        SkASSERT(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        SkASSERT(2 == visibilities.count() &&
                 kVertex_GrShaderFlag == visibilities[0] &&
                 kFragment_GrShaderFlag == visibilities[1]);
        // We set the visibility of the first binding to the vertex shader and the second to the
        // fragment shader.
        fBindingVisibilities.push_back(kVertex_GrShaderFlag);
        fBindingVisibilities.push_back(kFragment_GrShaderFlag);
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
        fPoolManager.getNewDescriptorSet(gpu, &vkDS);

        ds = new GrVkDescriptorSet(vkDS, fPoolManager.fPool, handle);
    }
    SkASSERT(ds);
    return ds;
}

void GrVkDescriptorSetManager::recycleDescriptorSet(const GrVkDescriptorSet* descSet) {
    SkASSERT(descSet);
    fFreeSets.push_back(descSet);
}

void GrVkDescriptorSetManager::release(const GrVkGpu* gpu) {
    fPoolManager.freeGPUResources(gpu);

    for (int i = 0; i < fFreeSets.count(); ++i) {
        fFreeSets[i]->unref(gpu);
    }
    fFreeSets.reset();
}

void GrVkDescriptorSetManager::abandon() {
    fPoolManager.abandonGPUResources();

    for (int i = 0; i < fFreeSets.count(); ++i) {
        fFreeSets[i]->unrefAndAbandon();
    }
    fFreeSets.reset();
}

bool GrVkDescriptorSetManager::isCompatible(VkDescriptorType type,
                                            const GrVkUniformHandler* uniHandler) const {
    SkASSERT(uniHandler);
    if (type != fPoolManager.fDescType) {
        return false;
    }

    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        if (fBindingVisibilities.count() != uniHandler->numSamplers()) {
            return false;
        }
        for (int i = 0; i < uniHandler->numSamplers(); ++i) {
            if (uniHandler->samplerVisibility(i) != fBindingVisibilities[i]) {
                return false;
            }
        }
    }
    return true;
}

bool GrVkDescriptorSetManager::isCompatible(VkDescriptorType type,
                                            const SkTArray<uint32_t>& visibilities) const {
    if (type != fPoolManager.fDescType) {
        return false;
    }

    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        if (fBindingVisibilities.count() != visibilities.count()) {
            return false;
        }
        for (int i = 0; i < visibilities.count(); ++i) {
            if (visibilities[i] != fBindingVisibilities[i]) {
                return false;
            }
        }
    }
    return true;
}

////////////////////////////////////////////////////////////////////////////////

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

GrVkDescriptorSetManager::DescriptorPoolManager::DescriptorPoolManager(
        VkDescriptorType type,
        GrVkGpu* gpu,
        const GrVkUniformHandler* uniformHandler)
    : fDescType(type)
    , fCurrentDescriptorCount(0)
    , fPool(nullptr) {
    this->init(gpu, type, uniformHandler, nullptr);
}

GrVkDescriptorSetManager::DescriptorPoolManager::DescriptorPoolManager(
        VkDescriptorType type,
        GrVkGpu* gpu,
        const SkTArray<uint32_t>& visibilities)
    : fDescType(type)
    , fCurrentDescriptorCount(0)
    , fPool(nullptr) {
    this->init(gpu, type, nullptr, &visibilities);
}

void GrVkDescriptorSetManager::DescriptorPoolManager::init(GrVkGpu* gpu,
                                                           VkDescriptorType type,
                                                           const GrVkUniformHandler* uniformHandler,
                                                           const SkTArray<uint32_t>* visibilities) {
    if (type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
        SkASSERT(SkToBool(uniformHandler) != SkToBool(visibilities));
        uint32_t numSamplers;
        if (uniformHandler) {
            numSamplers = (uint32_t)uniformHandler->numSamplers();
        } else {
            numSamplers = (uint32_t)visibilities->count();
        }

        std::unique_ptr<VkDescriptorSetLayoutBinding[]> dsSamplerBindings(
            new VkDescriptorSetLayoutBinding[numSamplers]);
        for (uint32_t i = 0; i < numSamplers; ++i) {
            uint32_t visibility;
            if (uniformHandler) {
                visibility = uniformHandler->samplerVisibility(i);
            } else {
                visibility = (*visibilities)[i];
            }
            dsSamplerBindings[i].binding = i;
            dsSamplerBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            dsSamplerBindings[i].descriptorCount = 1;
            dsSamplerBindings[i].stageFlags = visibility_to_vk_stage_flags(visibility);
            dsSamplerBindings[i].pImmutableSamplers = nullptr;
        }

        VkDescriptorSetLayoutCreateInfo dsSamplerLayoutCreateInfo;
        memset(&dsSamplerLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
        dsSamplerLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dsSamplerLayoutCreateInfo.pNext = nullptr;
        dsSamplerLayoutCreateInfo.flags = 0;
        dsSamplerLayoutCreateInfo.bindingCount = numSamplers;
        // Setting to nullptr fixes an error in the param checker validation layer. Even though
        // bindingCount is 0 (which is valid), it still tries to validate pBindings unless it is
        // null.
        dsSamplerLayoutCreateInfo.pBindings = numSamplers ? dsSamplerBindings.get() : nullptr;

        GR_VK_CALL_ERRCHECK(gpu->vkInterface(),
                            CreateDescriptorSetLayout(gpu->device(),
                                                      &dsSamplerLayoutCreateInfo,
                                                      nullptr,
                                                      &fDescLayout));
        fDescCountPerSet = numSamplers;
    } else {
        SkASSERT(type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
        // Create Uniform Buffer Descriptor
        // The vertex uniform buffer will have binding 0 and the fragment binding 1.
        VkDescriptorSetLayoutBinding dsUniBindings[kUniformDescPerSet];
        memset(&dsUniBindings, 0, 2 * sizeof(VkDescriptorSetLayoutBinding));
        dsUniBindings[0].binding = GrVkUniformHandler::kVertexBinding;
        dsUniBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dsUniBindings[0].descriptorCount = 1;
        dsUniBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        dsUniBindings[0].pImmutableSamplers = nullptr;
        dsUniBindings[1].binding = GrVkUniformHandler::kFragBinding;
        dsUniBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        dsUniBindings[1].descriptorCount = 1;
        dsUniBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        dsUniBindings[1].pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutCreateInfo uniformLayoutCreateInfo;
        memset(&uniformLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
        uniformLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uniformLayoutCreateInfo.pNext = nullptr;
        uniformLayoutCreateInfo.flags = 0;
        uniformLayoutCreateInfo.bindingCount = 2;
        uniformLayoutCreateInfo.pBindings = dsUniBindings;

        GR_VK_CALL_ERRCHECK(gpu->vkInterface(), CreateDescriptorSetLayout(gpu->device(),
                                                                          &uniformLayoutCreateInfo,
                                                                          nullptr,
                                                                          &fDescLayout));
        fDescCountPerSet = kUniformDescPerSet;
    }

    SkASSERT(fDescCountPerSet < kStartNumDescriptors);
    fMaxDescriptors = kStartNumDescriptors;
    SkASSERT(fMaxDescriptors > 0);
    this->getNewPool(gpu);
}

void GrVkDescriptorSetManager::DescriptorPoolManager::getNewPool(GrVkGpu* gpu) {
    if (fPool) {
        fPool->unref(gpu);
        uint32_t newPoolSize = fMaxDescriptors + ((fMaxDescriptors + 1) >> 1);
        if (newPoolSize < kMaxDescriptors) {
            fMaxDescriptors = newPoolSize;
        } else {
            fMaxDescriptors = kMaxDescriptors;
        }

    }
    fPool = gpu->resourceProvider().findOrCreateCompatibleDescriptorPool(fDescType,
                                                                         fMaxDescriptors);
    SkASSERT(fPool);
}

void GrVkDescriptorSetManager::DescriptorPoolManager::getNewDescriptorSet(GrVkGpu* gpu,
                                                                          VkDescriptorSet* ds) {
    if (!fMaxDescriptors) {
        return;
    }
    fCurrentDescriptorCount += fDescCountPerSet;
    if (fCurrentDescriptorCount > fMaxDescriptors) {
        this->getNewPool(gpu);
        fCurrentDescriptorCount = fDescCountPerSet;
    }

    VkDescriptorSetAllocateInfo dsAllocateInfo;
    memset(&dsAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    dsAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsAllocateInfo.pNext = nullptr;
    dsAllocateInfo.descriptorPool = fPool->descPool();
    dsAllocateInfo.descriptorSetCount = 1;
    dsAllocateInfo.pSetLayouts = &fDescLayout;
    GR_VK_CALL_ERRCHECK(gpu->vkInterface(), AllocateDescriptorSets(gpu->device(),
                                                                   &dsAllocateInfo,
                                                                   ds));
}

void GrVkDescriptorSetManager::DescriptorPoolManager::freeGPUResources(const GrVkGpu* gpu) {
    if (fDescLayout) {
        GR_VK_CALL(gpu->vkInterface(), DestroyDescriptorSetLayout(gpu->device(), fDescLayout,
                                                                  nullptr));
        fDescLayout = VK_NULL_HANDLE;
    }

    if (fPool) {
        fPool->unref(gpu);
        fPool = nullptr;
    }
}

void GrVkDescriptorSetManager::DescriptorPoolManager::abandonGPUResources() {
    fDescLayout = VK_NULL_HANDLE;
    if (fPool) {
        fPool->unrefAndAbandon();
        fPool = nullptr;
    }
}
