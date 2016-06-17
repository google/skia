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
    void initSimple(const GrVkGpu* gpu, const GrVkRenderTarget& target);

    struct AttachmentsDescriptor {
        struct AttachmentDesc {
            VkFormat fFormat;
            int fSamples;
            AttachmentDesc() : fFormat(VK_FORMAT_UNDEFINED), fSamples(0) {}
            bool operator==(const AttachmentDesc& right) const {
                return (fFormat == right.fFormat && fSamples == right.fSamples);
            }
            bool operator!=(const AttachmentDesc& right) const {
                return !(*this == right);
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

    VkRenderPass vkRenderPass() const { return fRenderPass; }

    void genKey(GrProcessorKeyBuilder* b) const;

private:
    GrVkRenderPass(const GrVkRenderPass&);
    GrVkRenderPass& operator=(const GrVkRenderPass&);

    void freeGPUData(const GrVkGpu* gpu) const override;

    VkRenderPass          fRenderPass;
    AttachmentFlags       fAttachmentFlags;
    AttachmentsDescriptor fAttachmentsDescriptor;

    typedef GrVkResource INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrVkRenderPass::AttachmentFlags);

#endif
