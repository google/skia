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

class GrVkAttachment;
class GrVkGpu;
class GrVkImageView;
class GrVkRenderPass;

class GrVkFramebuffer : public GrVkManagedResource {
public:
    static GrVkFramebuffer* Create(GrVkGpu* gpu,
                                   int width, int height,
                                   const GrVkRenderPass* renderPass,
                                   const GrVkAttachment* colorAttachment,
                                   const GrVkAttachment* resolveAttachment,
                                   const GrVkAttachment* stencilAttachment);

    VkFramebuffer framebuffer() const { return fFramebuffer; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkFramebuffer: %d (%d refs)\n", fFramebuffer, this->getRefCnt());
    }
#endif

private:
    GrVkFramebuffer(const GrVkGpu* gpu,
                    VkFramebuffer framebuffer,
                    sk_sp<GrVkAttachment> colorAttachment,
                    sk_sp<GrVkAttachment> resolveAttachment,
                    sk_sp<GrVkAttachment> stencilAttachment)
        : INHERITED(gpu)
        , fFramebuffer(framebuffer)
        , fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment))
        , fStencilAttachment(std::move(stencilAttachment)) {}

    ~GrVkFramebuffer() override;

    void freeGPUData() const override;

    VkFramebuffer  fFramebuffer;

    sk_sp<const GrVkAttachment> fColorAttachment;
    sk_sp<const GrVkAttachment> fResolveAttachment;
    sk_sp<const GrVkAttachment> fStencilAttachment;

    using INHERITED = GrVkManagedResource;
};

#endif
