/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED
#define skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED

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

// same as VULKAN_CALL but checks for success
#define VULKAN_CALL_ERRCHECK(IFACE, X)                   \
    VkResult SK_MACRO_APPEND_LINE(ret);                  \
    VULKAN_CALL_RESULT(IFACE, SK_MACRO_APPEND_LINE(ret), X)

#define VULKAN_CALL_RESULT_NOCHECK(IFACE, RESULT, X) \
    do {                                             \
        (RESULT) = VULKAN_CALL(IFACE, X);            \
    } while (false)


#endif // skgpu_graphite_VulkanGraphiteUtilsPriv_DEFINED
