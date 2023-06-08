/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"

#include "include/private/base/SkTArray.h"
#include "src/gpu/graphite/Attribute.h"
#include "src/gpu/graphite/vk/VulkanGraphicsPipeline.h"
#include "src/gpu/graphite/vk/VulkanGraphiteUtilsPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"

namespace skgpu::graphite {

static inline VkFormat attrib_type_to_vkformat(VertexAttribType type) {
    switch (type) {
        case VertexAttribType::kFloat:
            return VK_FORMAT_R32_SFLOAT;
        case VertexAttribType::kFloat2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexAttribType::kFloat3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexAttribType::kFloat4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexAttribType::kHalf:
            return VK_FORMAT_R16_SFLOAT;
        case VertexAttribType::kHalf2:
            return VK_FORMAT_R16G16_SFLOAT;
        case VertexAttribType::kHalf4:
            return VK_FORMAT_R16G16B16A16_SFLOAT;
        case VertexAttribType::kInt2:
            return VK_FORMAT_R32G32_SINT;
        case VertexAttribType::kInt3:
            return VK_FORMAT_R32G32B32_SINT;
        case VertexAttribType::kInt4:
            return VK_FORMAT_R32G32B32A32_SINT;
        case VertexAttribType::kByte:
            return VK_FORMAT_R8_SINT;
        case VertexAttribType::kByte2:
            return VK_FORMAT_R8G8_SINT;
        case VertexAttribType::kByte4:
            return VK_FORMAT_R8G8B8A8_SINT;
        case VertexAttribType::kUByte:
            return VK_FORMAT_R8_UINT;
        case VertexAttribType::kUByte2:
            return VK_FORMAT_R8G8_UINT;
        case VertexAttribType::kUByte4:
            return VK_FORMAT_R8G8B8A8_UINT;
        case VertexAttribType::kUByte_norm:
            return VK_FORMAT_R8_UNORM;
        case VertexAttribType::kUByte4_norm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case VertexAttribType::kShort2:
            return VK_FORMAT_R16G16_SINT;
        case VertexAttribType::kShort4:
            return VK_FORMAT_R16G16B16A16_SINT;
        case VertexAttribType::kUShort2:
            return VK_FORMAT_R16G16_UINT;
        case VertexAttribType::kUShort2_norm:
            return VK_FORMAT_R16G16_UNORM;
        case VertexAttribType::kInt:
            return VK_FORMAT_R32_SINT;
        case VertexAttribType::kUInt:
            return VK_FORMAT_R32_UINT;
        case VertexAttribType::kUShort_norm:
            return VK_FORMAT_R16_UNORM;
        case VertexAttribType::kUShort4_norm:
            return VK_FORMAT_R16G16B16A16_UNORM;
    }
    SK_ABORT("Unknown vertex attrib type");
}

static void setup_vertex_input_state(
        const SkSpan<const Attribute>& vertexAttrs,
        const SkSpan<const Attribute>& instanceAttrs,
        VkPipelineVertexInputStateCreateInfo* vertexInputInfo,
        skia_private::STArray<2, VkVertexInputBindingDescription, true>* bindingDescs,
        skia_private::STArray<16, VkVertexInputAttributeDescription>* attributeDescs,
        uint32_t maxVertexAttributes) {
    uint32_t totalAttributeCount = vertexAttrs.size() + instanceAttrs.size();
    SkASSERT(totalAttributeCount <= maxVertexAttributes);

    // Setup attribute & binding descriptions
    int attribIndex = 0;
    size_t vertexAttributeOffset = 0;
    for (auto attrib : vertexAttrs) {
        VkVertexInputAttributeDescription vkAttrib;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kVertexBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = vertexAttributeOffset;
        vertexAttributeOffset += attrib.sizeAlign4();
        attributeDescs->push_back(vkAttrib);
    }

    size_t instanceAttributeOffset = 0;
    for (auto attrib : instanceAttrs) {
        VkVertexInputAttributeDescription vkAttrib;
        vkAttrib.location = attribIndex++;
        vkAttrib.binding = VulkanGraphicsPipeline::kInstanceBufferIndex;
        vkAttrib.format = attrib_type_to_vkformat(attrib.cpuType());
        vkAttrib.offset = instanceAttributeOffset;
        instanceAttributeOffset += attrib.sizeAlign4();
        attributeDescs->push_back(vkAttrib);
    }

    if (vertexAttrs.size()) {
        bindingDescs->push_back() = {
                VulkanGraphicsPipeline::kVertexBufferIndex,
                (uint32_t) vertexAttributeOffset,
                VK_VERTEX_INPUT_RATE_VERTEX
        };
    }
    if (instanceAttrs.size()) {
        bindingDescs->push_back() = {
                VulkanGraphicsPipeline::kInstanceBufferIndex,
                (uint32_t) instanceAttributeOffset,
                VK_VERTEX_INPUT_RATE_INSTANCE
        };
    }

    memset(vertexInputInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo->pNext = nullptr;
    vertexInputInfo->flags = 0;
    vertexInputInfo->vertexBindingDescriptionCount = bindingDescs->size();
    vertexInputInfo->pVertexBindingDescriptions = bindingDescs->begin();
    vertexInputInfo->vertexAttributeDescriptionCount = attributeDescs->size();
    vertexInputInfo->pVertexAttributeDescriptions = attributeDescs->begin();
}

sk_sp<VulkanGraphicsPipeline> VulkanGraphicsPipeline::Make(
        const VulkanSharedContext* sharedContext,
        VkShaderModule vertexShader,
        SkSpan<const Attribute> vertexAttrs,
        SkSpan<const Attribute> instanceAttrs,
        VkShaderModule fragShader) {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    skia_private::STArray<2, VkVertexInputBindingDescription, true> bindingDescs;
    skia_private::STArray<16, VkVertexInputAttributeDescription> attributeDescs;

    setup_vertex_input_state(vertexAttrs,
                             instanceAttrs,
                             &vertexInputInfo,
                             &bindingDescs,
                             &attributeDescs,
                             sharedContext->vulkanCaps().maxVertexAttributes());

    // TODO: Set up other helpers and structs to populate VkGraphicsPipelineCreateInfo.

    // After setting modules in VkPipelineShaderStageCreateInfo, we can clean them up.
    VULKAN_CALL(sharedContext->interface(),
                DestroyShaderModule(sharedContext->device(), vertexShader, nullptr));
    if (fragShader != VK_NULL_HANDLE) {
        VULKAN_CALL(sharedContext->interface(),
                    DestroyShaderModule(sharedContext->device(), fragShader, nullptr));
    }

    return sk_sp<VulkanGraphicsPipeline>(new VulkanGraphicsPipeline(sharedContext));
}

void VulkanGraphicsPipeline::freeGpuData() {
}

} // namespace skgpu::graphite
