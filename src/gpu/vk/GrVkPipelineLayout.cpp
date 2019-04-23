/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkPipelineLayout.h"
#include "src/gpu/vk/GrVkUtil.h"

void GrVkPipelineLayout::freeGPUData(GrVkGpu* gpu) const {
    GR_VK_CALL(gpu->vkInterface(), DestroyPipelineLayout(gpu->device(), fPipelineLayout, nullptr));
}
