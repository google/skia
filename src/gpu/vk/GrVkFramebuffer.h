/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkFramebuffer_DEFINED
#define GrVkFramebuffer_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkManagedResource.h"

class GrVkGpu;
class GrVkImageView;
class GrVkRenderPass;

class GrVkFramebuffer : public GrVkManagedResource {
public:
    static GrVkFramebuffer* Create(GrVkGpu* gpu,
                                   int width, int height,
                                   const GrVkRenderPass* renderPass,
                                   const GrVkImageView* colorAttachment,
                                   const GrVkImageView* resolveAttachment,
                                   const GrVkImageView* stencilAttachment);

    VkFramebuffer framebuffer() const { return fFramebuffer; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkFramebuffer: %d (%d refs)\n", fFramebuffer, this->getRefCnt());
    }
#endif

private:
    GrVkFramebuffer(const GrVkGpu* gpu, VkFramebuffer framebuffer)
        : INHERITED(gpu), fFramebuffer(framebuffer) {}

    GrVkFramebuffer(const GrVkFramebuffer&);
    GrVkFramebuffer& operator=(const GrVkFramebuffer&);

    void freeGPUData() const override;

    VkFramebuffer  fFramebuffer;

    using INHERITED = GrVkManagedResource;
};

#endif
