/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanUtils_DEFINED
#define skgpu_graphite_VulkanUtils_DEFINED

#include "include/gpu/vk/VulkanTypes.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/vk/VulkanInterface.h"

// Helper macros to call functions on the VulkanInterface
#define VULKAN_CALL(IFACE, X) (IFACE)->fFunctions.f##X

// TODO: This needs to add checks for device lost on calls. See Ganesh version
#define VULKAN_CALL_RESULT(IFACE, RESULT, X)                               \
    do {                                                                   \
        (RESULT) = VULKAN_CALL(IFACE, X);                                  \
        SkASSERT(VK_SUCCESS == RESULT || VK_ERROR_DEVICE_LOST == RESULT);  \
        if (RESULT != VK_SUCCESS) {                                        \
            SKGPU_LOG_E("Failed vulkan call. Error: %d," #X "\n", RESULT); \
        }                                                                  \
    } while (false)

#endif // skgpu_graphite_VulkanUtils_DEFINED
