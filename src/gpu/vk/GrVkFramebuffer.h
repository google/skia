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

#include <cinttypes>

class GrVkAttachment;
class GrVkGpu;
class GrVkImageView;
class GrVkRenderPass;

class GrVkFramebuffer : public GrVkManagedResource {
public:
    static sk_sp<const GrVkFramebuffer> Make(GrVkGpu* gpu,
                                             SkISize dimensions,
                                             sk_sp<const GrVkRenderPass> compatibleRenderPass,
                                             GrVkAttachment* colorAttachment,
                                             GrVkAttachment* resolveAttachment,
                                             GrVkAttachment* stencilAttachment,
                                             GrVkResourceProvider::CompatibleRPHandle);

    // Used for wrapped external secondary command buffers
    GrVkFramebuffer(const GrVkGpu* gpu,
                    sk_sp<GrVkAttachment> colorAttachment,
                    sk_sp<const GrVkRenderPass> renderPass,
                    std::unique_ptr<GrVkSecondaryCommandBuffer>);

    VkFramebuffer framebuffer() const {
        SkASSERT(!this->isExternal());
        return fFramebuffer;
    }

    bool isExternal() const { return fExternalRenderPass.get(); }
    const GrVkRenderPass* externalRenderPass() const { return fExternalRenderPass.get(); }
    std::unique_ptr<GrVkSecondaryCommandBuffer> externalCommandBuffer();

    // When we wrap a secondary command buffer, we will record GrManagedResources onto it which need
    // to be kept alive till the command buffer gets submitted and the GPU has finished. However, in
    // the wrapped case, we don't know when the command buffer gets submitted and when it is
    // finished on the GPU since the client is in charge of that. However, we do require that the
    // client keeps the GrVkSecondaryCBDrawContext alive and call releaseResources on it once the
    // GPU is finished all the work. Thus we can use this to manage the lifetime of our
    // GrVkSecondaryCommandBuffers. By storing them on the external GrVkFramebuffer owned by the
    // GrVkRenderTarget, which is owned by the SkGpuDevice on the GrVkSecondaryCBDrawContext, we
    // assure that the GrManagedResources held by the GrVkSecondaryCommandBuffer don't get deleted
    // before they are allowed to.
    void returnExternalGrSecondaryCommandBuffer(std::unique_ptr<GrVkSecondaryCommandBuffer>);

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkFramebuffer: %" PRIdPTR " (%d refs)\n",
                 (intptr_t)fFramebuffer, this->getRefCnt());
    }
#endif

    const GrVkRenderPass* compatibleRenderPass() const { return fCompatibleRenderPass.get(); }

    GrVkResourceProvider::CompatibleRPHandle compatibleRenderPassHandle() const {
        return fCompatibleRenderPassHandle;
    }

    GrVkAttachment* colorAttachment() { return fColorAttachment.get(); }
    GrVkAttachment* resolveAttachment() { return fResolveAttachment.get(); }
    GrVkAttachment* stencilAttachment() { return fStencilAttachment.get(); }

private:
    GrVkFramebuffer(const GrVkGpu* gpu,
                    VkFramebuffer framebuffer,
                    sk_sp<GrVkAttachment> colorAttachment,
                    sk_sp<GrVkAttachment> resolveAttachment,
                    sk_sp<GrVkAttachment> stencilAttachment,
                    sk_sp<const GrVkRenderPass> compatibleRenderPass,
                    GrVkResourceProvider::CompatibleRPHandle);

    ~GrVkFramebuffer() override;

    void freeGPUData() const override;
    void releaseResources();

    VkFramebuffer  fFramebuffer = VK_NULL_HANDLE;

    sk_sp<GrVkAttachment> fColorAttachment;
    sk_sp<GrVkAttachment> fResolveAttachment;
    sk_sp<GrVkAttachment> fStencilAttachment;

    sk_sp<const GrVkRenderPass> fCompatibleRenderPass;
    GrVkResourceProvider::CompatibleRPHandle fCompatibleRenderPassHandle;

    sk_sp<const GrVkRenderPass> fExternalRenderPass;
    std::unique_ptr<GrVkSecondaryCommandBuffer> fExternalCommandBuffer;
};

#endif
