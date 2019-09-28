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
#include "src/gpu/vk/GrVkResource.h"

class GrVkGpu;
class GrVkImageView;
class GrVkRenderPass;

class GrVkFramebuffer : public GrVkResource {
public:
    static GrVkFramebuffer* Create(GrVkGpu* gpu,
                                   int width, int height,
                                   const GrVkRenderPass* renderPass,
                                   const GrVkImageView* colorAttachment,
                                   const GrVkImageView* stencilAttachment);

    VkFramebuffer framebuffer() const { return fFramebuffer; }

#ifdef SK_TRACE_VK_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkFramebuffer: %d (%d refs)\n", fFramebuffer, this->getRefCnt());
    }
#endif

private:
    GrVkFramebuffer(VkFramebuffer framebuffer) : INHERITED(), fFramebuffer(framebuffer) {}

    GrVkFramebuffer(const GrVkFramebuffer&);
    GrVkFramebuffer& operator=(const GrVkFramebuffer&);

    void freeGPUData(GrVkGpu* gpu) const override;

    VkFramebuffer  fFramebuffer;

    typedef GrVkResource INHERITED;
};

#endif
