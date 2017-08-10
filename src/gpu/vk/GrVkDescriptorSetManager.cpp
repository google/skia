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

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateUniformManager(GrVkGpu* gpu) {
    SkSTArray<2, uint32_t> visibilities;
    // We set the visibility of the first binding to all supported geometry processing shader
    // stages (vertex, tesselation, geometry, etc.) and the second binding to the fragment
    // shader.
    uint32_t geomStages = kVertex_GrShaderFlag;
    if (gpu->vkCaps().shaderCaps()->geometryShaderSupport()) {
        geomStages |= kGeometry_GrShaderFlag;
    }
    visibilities.push_back(geomStages);
    visibilities.push_back(kFragment_GrShaderFlag);
    return new GrVkDescriptorSetManager(gpu, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, visibilities);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateSamplerManager(
        GrVkGpu* gpu, VkDescriptorType type, const GrVkUniformHandler& uniformHandler) {
    SkSTArray<4, uint32_t> visibilities;
    if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type) {
        for (int i = 0 ; i < uniformHandler.numSamplers(); ++i) {
            visibilities.push_back(uniformHandler.samplerVisibility(i));
        }
    } else {
        SkASSERT(type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
        for (int i = 0 ; i < uniformHandler.numTexelBuffers(); ++i) {
            visibilities.push_back(uniformHandler.texelBufferVisibility(i));
        }
    }
    return CreateSamplerManager(gpu, type, visibilities);
}

GrVkDescriptorSetManager* GrVkDescriptorSetManager::CreateSamplerManager(
        GrVkGpu* gpu, VkDescriptorType type, const SkTArray<uint32_t>& visibilities) {
    return new GrVkDescriptorSetManager(gpu, type, visibilities);
}

GrVkDescriptorSetManager::GrVkDescriptorSetManager(GrVkGpu* gpu,
                                                   VkDescriptorType type,
                                                   const SkTArray<uint32_t>& visibilities)
    : fPoolManager(type, gpu, visibilities) {
    for (int i = 0; i < visibilities.count(); ++i) {
        fBindingVisibilities.push_back(visibilities[i]);
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
    } else if (VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER == type) {
        if (fBindingVisibilities.count() != uniHandler->numTexelBuffers()) {
            return false;
        }
        for (int i = 0; i < uniHandler->numTexelBuffers(); ++i) {
            if (uniHandler->texelBufferVisibility(i) != fBindingVisibilities[i]) {
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

    if (VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER == type ||
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER == type) {
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
        const SkTArray<uint32_t>& visibilities)
    : fDescType(type)
    , fCurrentDescriptorCount(0)
    , fPool(nullptr) {


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
            dsSamplerBindings[i].pImmutableSamplers = nullptr;
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

        GR_VK_CALL_ERRCHECK(gpu->vkInterface(),
                            CreateDescriptorSetLayout(gpu->device(),
                                                      &dsSamplerLayoutCreateInfo,
                                                      nullptr,
                                                      &fDescLayout));
        fDescCountPerSet = visibilities.count();
    } else {
        SkASSERT(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER == type);
        GR_STATIC_ASSERT(2 == kUniformDescPerSet);
        SkASSERT(kUniformDescPerSet == visibilities.count());
        // Create Uniform Buffer Descriptor
        static const uint32_t bindings[kUniformDescPerSet] =
                { GrVkUniformHandler::kGeometryBinding, GrVkUniformHandler::kFragBinding };
        VkDescriptorSetLayoutBinding dsUniBindings[kUniformDescPerSet];
        memset(&dsUniBindings, 0, kUniformDescPerSet * sizeof(VkDescriptorSetLayoutBinding));
        for (int i = 0; i < kUniformDescPerSet; ++i) {
            dsUniBindings[i].binding = bindings[i];
            dsUniBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            dsUniBindings[i].descriptorCount = 1;
            dsUniBindings[i].stageFlags = visibility_to_vk_stage_flags(visibilities[i]);
            dsUniBindings[i].pImmutableSamplers = nullptr;
        }

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
