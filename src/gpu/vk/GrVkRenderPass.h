/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkRenderPass_DEFINED
#define GrVkRenderPass_DEFINED

#include "GrTypes.h"

#include "GrVkResource.h"

#include "vk/GrVkDefines.h"

class GrProcessorKeyBuilder;
class GrVkGpu;
class GrVkRenderTarget;

class GrVkRenderPass : public GrVkResource {
public:
    GrVkRenderPass() : INHERITED(), fRenderPass(VK_NULL_HANDLE) {}

    struct LoadStoreOps {
        VkAttachmentLoadOp  fLoadOp;
        VkAttachmentStoreOp fStoreOp;

        LoadStoreOps(VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp)
            : fLoadOp(loadOp)
            , fStoreOp(storeOp) {}

        bool operator==(const LoadStoreOps& right) const {
            return fLoadOp == right.fLoadOp && fStoreOp == right.fStoreOp;
        }

        bool operator!=(const LoadStoreOps& right) const {
            return !(*this == right);
        }
    };

    void initSimple(const GrVkGpu* gpu, const GrVkRenderTarget& target);
    void init(const GrVkGpu* gpu,
              const GrVkRenderTarget& target,
              const LoadStoreOps& colorOp,
              const LoadStoreOps& resolveOp,
              const LoadStoreOps& stencilOp);

    void init(const GrVkGpu* gpu,
              const GrVkRenderPass& compatibleRenderPass,
              const LoadStoreOps& colorOp,
              const LoadStoreOps& resolveOp,
              const LoadStoreOps& stencilOp);

    struct AttachmentsDescriptor {
        struct AttachmentDesc {
            VkFormat fFormat;
            int fSamples;
            LoadStoreOps fLoadStoreOps;

            AttachmentDesc()
                : fFormat(VK_FORMAT_UNDEFINED)
                , fSamples(0)
                , fLoadStoreOps(VK_ATTACHMENT_LOAD_OP_LOAD, VK_ATTACHMENT_STORE_OP_STORE) {}
            bool operator==(const AttachmentDesc& right) const {
                return (fFormat == right.fFormat &&
                        fSamples == right.fSamples &&
                        fLoadStoreOps == right.fLoadStoreOps);
            }
            bool operator!=(const AttachmentDesc& right) const {
                return !(*this == right);
            }
            bool isCompatible(const AttachmentDesc& desc) const {
                return (fFormat == desc.fFormat && fSamples == desc.fSamples);
            }
        };
        AttachmentDesc fColor;
        AttachmentDesc fResolve;
        AttachmentDesc fStencil;
        uint32_t       fAttachmentCount;
    };

    enum AttachmentFlags {
        kColor_AttachmentFlag = 0x1,
        kResolve_AttachmentFlag = 0x2,
        kStencil_AttachmentFlag = 0x4,
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(AttachmentFlags);

    // The following return the index of the render pass attachment array for the given attachment.
    // If the render pass does not have the given attachment it will return false and not set the
    // index value.
    bool colorAttachmentIndex(uint32_t* index) const;
    bool resolveAttachmentIndex(uint32_t* index) const;
    bool stencilAttachmentIndex(uint32_t* index) const;

    // Sets the VkRenderPassBeginInfo and VkRenderPassContents need to begin a render pass.
    // TODO: In the future I expect this function will also take an optional render area instead of
    // defaulting to the entire render target.
    // TODO: Figure out if load clear values should be passed into this function or should be stored
    // on the GrVkRenderPass at create time since we'll know at that point if we want to do a load
    // clear.
    void getBeginInfo(const GrVkRenderTarget& target,
                      VkRenderPassBeginInfo* beginInfo,
                      VkSubpassContents* contents) const;

    // Returns whether or not the structure of a RenderTarget matches that of the VkRenderPass in
    // this object. Specifically this compares that the number of attachments, format of
    // attachments, and sample counts are all the same. This function is used in the creation of
    // basic RenderPasses that can be used when creating a VkFrameBuffer object.
    bool isCompatible(const GrVkRenderTarget& target) const;

    bool isCompatible(const GrVkRenderPass& renderPass) const;

    bool equalLoadStoreOps(const LoadStoreOps& colorOps,
                           const LoadStoreOps& resolveOps,
                           const LoadStoreOps& stencilOps) const;

    VkRenderPass vkRenderPass() const { return fRenderPass; }

    void genKey(GrProcessorKeyBuilder* b) const;

private:
    GrVkRenderPass(const GrVkRenderPass&);

    void init(const GrVkGpu* gpu,
              const LoadStoreOps& colorOps,
              const LoadStoreOps& resolveOps,
              const LoadStoreOps& stencilOps);

    bool isCompatible(const AttachmentsDescriptor&, const AttachmentFlags&) const;

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkRenderPass          fRenderPass;
    AttachmentFlags       fAttachmentFlags;
    AttachmentsDescriptor fAttachmentsDescriptor;

    typedef GrVkResource INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrVkRenderPass::AttachmentFlags);

#endif
