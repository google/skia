
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkDefines_DEFINED
#define GrVkDefines_DEFINED

#include "SkTypes.h"

#ifdef SK_VULKAN

#ifdef SK_VULKAN_HEADER
#include SK_VULKAN_HEADER // IWYU pragma: export
#else
// This is deprecated and all clients should define their own custum header shim that sets up
// defines and includes the vulkan.h header. Then they should define SK_VULKAN_HEADER or set the
// skia_vulkan_header in gn to point to their custom header.
#  if defined(SK_BUILD_FOR_WIN)
#     if !defined(VK_USE_PLATFORM_WIN32_KHR)
#        define VK_USE_PLATFORM_WIN32_KHR
#     endif
#  elif defined(SK_BUILD_FOR_ANDROID)
#     if !defined(VK_USE_PLATFORM_ANDROID_KHR)
#        define VK_USE_PLATFORM_ANDROID_KHR
#     endif
#  elif defined(SK_BUILD_FOR_UNIX)
#     if defined(__Fuchsia__)
#       if !defined(VK_USE_PLATFORM_MAGMA_KHR)
#         define VK_USE_PLATFORM_MAGMA_KHR
#       endif
#     else
#       if !defined(VK_USE_PLATFORM_XCB_KHR)
#          define VK_USE_PLATFORM_XCB_KHR
#       endif
#     endif
#  endif

// We create our own function table and never directly call any functions via vk*(). So no need to
// include the prototype functions.
#  if !defined(VK_NO_PROTOTYPES) && !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
#    define VK_NO_PROTOTYPES
#  endif

#  include <vulkan/vulkan.h> // IWYU pragma: export
#endif // SK_VULKAN_HEADER

#define SKIA_REQUIRED_VULKAN_HEADER_VERSION 17
#if VK_HEADER_VERSION < SKIA_REQUIRED_VULKAN_HEADER_VERSION
#error "Vulkan header version is too low"
#endif

// The AMD VulkanMemoryAllocator needs the objects from this extension to be declared.
#ifndef VK_KHR_get_memory_requirements2

#define VK_KHR_get_memory_requirements2 1
#define VK_KHR_GET_MEMORY_REQUIREMENTS_2_SPEC_VERSION 1
#define VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME "VK_KHR_get_memory_requirements2"

typedef struct VkBufferMemoryRequirementsInfo2KHR {
    VkStructureType    sType;
    const void*        pNext;
    VkBuffer           buffer;
} VkBufferMemoryRequirementsInfo2KHR;

typedef struct VkImageMemoryRequirementsInfo2KHR {
    VkStructureType    sType;
    const void*        pNext;
    VkImage            image;
} VkImageMemoryRequirementsInfo2KHR;

typedef struct VkImageSparseMemoryRequirementsInfo2KHR {
    VkStructureType    sType;
    const void*        pNext;
    VkImage            image;
} VkImageSparseMemoryRequirementsInfo2KHR;

typedef struct VkMemoryRequirements2KHR {
    VkStructureType         sType;
    void*                   pNext;
    VkMemoryRequirements    memoryRequirements;
} VkMemoryRequirements2KHR;

typedef struct VkSparseImageMemoryRequirements2KHR {
    VkStructureType                    sType;
    void*                              pNext;
    VkSparseImageMemoryRequirements    memoryRequirements;
} VkSparseImageMemoryRequirements2KHR;


typedef void (VKAPI_PTR *PFN_vkGetImageMemoryRequirements2KHR)(VkDevice device, const VkImageMemoryRequirementsInfo2KHR* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements);
typedef void (VKAPI_PTR *PFN_vkGetBufferMemoryRequirements2KHR)(VkDevice device, const VkBufferMemoryRequirementsInfo2KHR* pInfo, VkMemoryRequirements2KHR* pMemoryRequirements);
typedef void (VKAPI_PTR *PFN_vkGetImageSparseMemoryRequirements2KHR)(VkDevice device, const VkImageSparseMemoryRequirementsInfo2KHR* pInfo, uint32_t* pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2KHR* pSparseMemoryRequirements);

static constexpr VkStructureType VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2_KHR = (VkStructureType) 1000146000;
static constexpr VkStructureType VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2_KHR = (VkStructureType) 1000146001;
static constexpr VkStructureType VK_STRUCTURE_TYPE_IMAGE_SPARSE_MEMORY_REQUIREMENTS_INFO_2_KHR = (VkStructureType) 1000146002;
static constexpr VkStructureType VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR = (VkStructureType) 1000146003;
static constexpr VkStructureType VK_STRUCTURE_TYPE_SPARSE_IMAGE_MEMORY_REQUIREMENTS_2_KHR = (VkStructureType) 1000146004;

#endif // VK_KHR_get_memory_requirements2

// Also needed for VulkanMemoryAllocator
#ifndef VK_KHR_dedicated_allocation

#define VK_KHR_dedicated_allocation 1
#define VK_KHR_DEDICATED_ALLOCATION_SPEC_VERSION 3
#define VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME "VK_KHR_dedicated_allocation"

typedef struct VkMemoryDedicatedRequirementsKHR {
    VkStructureType    sType;
    void*              pNext;
    VkBool32           prefersDedicatedAllocation;
    VkBool32           requiresDedicatedAllocation;
} VkMemoryDedicatedRequirementsKHR;

typedef struct VkMemoryDedicatedAllocateInfoKHR {
    VkStructureType    sType;
    const void*        pNext;
    VkImage            image;
    VkBuffer           buffer;
} VkMemoryDedicatedAllocateInfoKHR;

static constexpr VkStructureType VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS_KHR = (VkStructureType) 1000127000;
static constexpr VkStructureType VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO_KHR = (VkStructureType) 1000127001;

#endif // VK_KHR_dedicated_allocation

#endif

#endif
