/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkBufferView.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkUtil.h"

const GrVkBufferView* GrVkBufferView::Create(const GrVkGpu* gpu, VkBuffer buffer, VkFormat format,
                                             VkDeviceSize offset, VkDeviceSize range) {
    VkBufferView bufferView;

    // Create the VkBufferView
    VkBufferViewCreateInfo viewInfo;
    memset(&viewInfo, 0, sizeof(VkBufferViewCreateInfo));
    viewInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    viewInfo.pNext = nullptr;
    viewInfo.flags = 0;
    viewInfo.buffer = buffer;
    viewInfo.format = format;
    viewInfo.offset = offset;
    viewInfo.range = range;

    VkResult err = GR_VK_CALL(gpu->vkInterface(), CreateBufferView(gpu->device(), &viewInfo,
                                                                   nullptr, &bufferView));
    if (err) {
        return nullptr;
    }

    return new GrVkBufferView(bufferView);
}

void GrVkBufferView::freeGPUData(GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(), DestroyBufferView(gpu->device(), fBufferView, nullptr));
}
