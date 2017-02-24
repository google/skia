
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

#include "GrTypes.h"
#include "vk/GrVkDefines.h"

/**
 * KHR_debug
 */
/*typedef void (GR_GL_FUNCTION_TYPE* GrVkDEBUGPROC)(GrVkenum source,
                                                  GrVkenum type,
                                                  GrVkuint id,
                                                  GrVkenum severity,
                                                  GrVksizei length,
                                                  const GrVkchar* message,
                                                  const void* userParam);*/



///////////////////////////////////////////////////////////////////////////////
/**
 * Types for interacting with Vulkan resources created externally to Skia. GrBackendObjects for
 * Vulkan textures are really const GrVkImageInfo*
 */
struct GrVkAlloc {
    VkDeviceMemory fMemory;  // can be VK_NULL_HANDLE iff Tex is an RT and uses borrow semantics
    VkDeviceSize   fOffset;
    VkDeviceSize   fSize;    // this can be indeterminate iff Tex uses borrow semantics
    uint32_t       fFlags;

    enum Flag {
        kNoncoherent_Flag = 0x1,   // memory must be flushed to device after mapping
    };
};

struct GrVkImageInfo {
    /**
     * If the image's format is sRGB (GrVkFormatIsSRGB returns true), then the image must have
     * been created with VkImageCreateFlags containing VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.
     */
    VkImage        fImage;
    GrVkAlloc      fAlloc;
    VkImageTiling  fImageTiling;
    VkImageLayout  fImageLayout;
    VkFormat       fFormat;
    uint32_t       fLevelCount;

    // This gives a way for a client to update the layout of the Image if they change the layout
    // while we're still holding onto the wrapped texture. They will first need to get a handle
    // to our internal GrVkImageInfo by calling getTextureHandle on a GrVkTexture.
    void updateImageLayout(VkImageLayout layout) { fImageLayout = layout; }
};

GR_STATIC_ASSERT(sizeof(GrBackendObject) >= sizeof(const GrVkImageInfo*));

#endif
