
/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkTypes_DEFINED
#define GrVkTypes_DEFINED

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
 * Vulkan textures are really const GrVkTextureInfo*
 */

struct GrVkTextureInfo {
    VkImage        fImage;
    VkDeviceMemory fAlloc;    // this may be null iff the texture is an RT and uses borrow semantics
    VkImageTiling  fImageTiling;
    VkImageLayout  fImageLayout;
};

GR_STATIC_ASSERT(sizeof(GrBackendObject) >= sizeof(const GrVkTextureInfo*));

#endif
