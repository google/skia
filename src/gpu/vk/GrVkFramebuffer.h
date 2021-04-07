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
#include "src/gpu/vk/GrVkResourceProvider.h"

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
                                   const GrVkAttachment* stencilAttachment,
                                   GrVkResourceProvider::CompatibleRPHandle);

    VkFramebuffer framebuffer() const { return fFramebuffer; }

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkFramebuffer: %d (%d refs)\n", fFramebuffer, this->getRefCnt());
    }
#endif

    GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle() const {
        return fCompatibleRenderPassHandle;
    }

private:
    GrVkFramebuffer(const GrVkGpu* gpu,
                    VkFramebuffer framebuffer,
                    sk_sp<GrVkAttachment> colorAttachment,
                    sk_sp<GrVkAttachment> resolveAttachment,
                    sk_sp<GrVkAttachment> stencilAttachment,
                    GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle)
        : INHERITED(gpu)
        , fFramebuffer(framebuffer)
        , fColorAttachment(std::move(colorAttachment))
        , fResolveAttachment(std::move(resolveAttachment))
        , fStencilAttachment(std::move(stencilAttachment))
        , fCompatibleRenderPassHandle(compatibleRenderPassHandle) {}

    ~GrVkFramebuffer() override;

    void freeGPUData() const override;

    VkFramebuffer  fFramebuffer;

    sk_sp<const GrVkAttachment> fColorAttachment;
    sk_sp<const GrVkAttachment> fResolveAttachment;
    sk_sp<const GrVkAttachment> fStencilAttachment;

    GrVkResourceProvider::CompatibleRPHandle fCompatibleRenderPassHandle;

    using INHERITED = GrVkManagedResource;
};

#endif
