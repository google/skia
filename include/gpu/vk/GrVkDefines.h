
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDefines_DEFINED
#define GrVkDefines_DEFINED

#ifdef SK_VULKAN

#if defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_WIN32)
#   if !defined(VK_USE_PLATFORM_WIN32_KHR)
#      define VK_USE_PLATFORM_WIN32_KHR
#   endif
#elif defined(SK_BUILD_FOR_ANDROID)
#   if !defined(VK_USE_PLATFORM_ANDROID_KHR)
#      define VK_USE_PLATFORM_ANDROID_KHR
#   endif
#elif defined(SK_BUILD_FOR_UNIX)
#   if defined(__Fuchsia__)
#     if !defined(VK_USE_PLATFORM_MAGMA_KHR)
#       define VK_USE_PLATFORM_MAGMA_KHR
#     endif
#   else
#     if !defined(VK_USE_PLATFORM_XCB_KHR)
#        define VK_USE_PLATFORM_XCB_KHR
#     endif
#   endif
#endif

#include <vulkan/vulkan.h>

#define SKIA_REQUIRED_VULKAN_HEADER_VERSION 17
#if VK_HEADER_VERSION < SKIA_REQUIRED_VULKAN_HEADER_VERSION
#error "Vulkan header version is too low"
#endif

#ifndef VK_EXT_discard_rectangles

// Installed Vulkan SDK is too old to define discard rectangles: define them here.
#define VK_EXT_discard_rectangles 1
#define VK_EXT_DISCARD_RECTANGLES_SPEC_VERSION 1
#define VK_EXT_DISCARD_RECTANGLES_EXTENSION_NAME "VK_EXT_discard_rectangles"


typedef enum VkDiscardRectangleModeEXT {
    VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT = 0,
    VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT = 1,
    VK_DISCARD_RECTANGLE_MODE_BEGIN_RANGE_EXT = VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT,
    VK_DISCARD_RECTANGLE_MODE_END_RANGE_EXT = VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT,
    VK_DISCARD_RECTANGLE_MODE_RANGE_SIZE_EXT = (VK_DISCARD_RECTANGLE_MODE_EXCLUSIVE_EXT - VK_DISCARD_RECTANGLE_MODE_INCLUSIVE_EXT + 1),
    VK_DISCARD_RECTANGLE_MODE_MAX_ENUM_EXT = 0x7FFFFFFF
} VkDiscardRectangleModeEXT;

typedef VkFlags VkPipelineDiscardRectangleStateCreateFlagsEXT;

typedef struct VkPhysicalDeviceDiscardRectanglePropertiesEXT {
    VkStructureType    sType;
    const void*        pNext;
    uint32_t           maxDiscardRectangles;
} VkPhysicalDeviceDiscardRectanglePropertiesEXT;

typedef struct VkPipelineDiscardRectangleStateCreateInfoEXT {
    VkStructureType                                  sType;
    const void*                                      pNext;
    VkPipelineDiscardRectangleStateCreateFlagsEXT    flags;
    VkDiscardRectangleModeEXT                        discardRectangleMode;
    uint32_t                                         discardRectangleCount;
    const VkRect2D*                                  pDiscardRectangles;
} VkPipelineDiscardRectangleStateCreateInfoEXT;


typedef void (VKAPI_PTR *PFN_vkCmdSetDiscardRectangleEXT)(VkCommandBuffer commandBuffer, uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, const VkRect2D* pDiscardRectangles);

#endif // !VK_EXT_discard_rectangles

#endif // SK_VULKAN

#endif // GrVkDefines_DEFINED
