/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrVkBackendContext_DEFINED
#define GrVkBackendContext_DEFINED

#include "SkRefCnt.h"

#include "vulkan/vulkan.h"

#ifdef SK_DEBUG
#define ENABLE_VK_LAYERS
#endif

struct GrVkInterface;

// The BackendContext contains all of the base Vulkan objects needed by the GrVkGpu. The assumption
// is that the client will set these up and pass them to the GrVkGpu constructor. The VkDevice
// created must support at least one graphics queue, which is passed in as well. 
// The QueueFamilyIndex must match the family of the given queue. It is needed for CommandPool 
// creation, and any GrBackendObjects handed to us (e.g., for wrapped textures) need to be created
// in or transitioned to that family.
struct GrVkBackendContext : public SkRefCnt {
    VkInstance                        fInstance;
    VkPhysicalDevice                  fPhysicalDevice;
    VkDevice                          fDevice;
    VkQueue                           fQueue;
    uint32_t                          fQueueFamilyIndex;
    SkAutoTUnref<const GrVkInterface> fInterface;

    // Helper function to create the default Vulkan objects needed by the GrVkGpu object
    static const GrVkBackendContext* Create();

    ~GrVkBackendContext() override;
};

#endif
