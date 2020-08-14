/*
* Copyright 2015 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkRenderPass_DEFINED
#define GrVkRenderPass_DEFINED

#include "include/gpu/GrTypes.h"
#include "include/gpu/vk/GrVkTypes.h"
#include "src/gpu/vk/GrVkManagedResource.h"

class GrProcessorKeyBuilder;
class GrVkGpu;
class GrVkRenderTarget;

class GrVkRenderPass : public GrVkManagedResource {
public:
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

    // Used when importing an external render pass. In this case we have to explicitly be told the
    // color attachment index
    explicit GrVkRenderPass(const GrVkGpu* gpu, VkRenderPass renderPass,
                            uint32_t colorAttachmentIndex)
            : INHERITED(gpu)
            , fRenderPass(renderPass)
            , fAttachmentFlags(kExternal_AttachmentFlag)
            , fHasSelfDependency(false)
            , fClearValueCount(0)
            , fColorAttachmentIndex(colorAttachmentIndex) {}

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
        AttachmentDesc fStencil;
        uint32_t       fAttachmentCount;
    };

    enum AttachmentFlags {
        kColor_AttachmentFlag = 0x1,
        kStencil_AttachmentFlag = 0x2,
        // The external attachment flag signals that this render pass is imported from an external
        // client. Since we don't know every attachment on the render pass we don't set any of the
        // specific attachment flags when using external. However, the external render pass must
        // at least have a color attachment.
        kExternal_AttachmentFlag = 0x4,
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(AttachmentFlags);

    static GrVkRenderPass* CreateSimple(GrVkGpu*,
                                        AttachmentsDescriptor*,
                                        AttachmentFlags,
                                        bool needsSelfDependency);
    static GrVkRenderPass* Create(GrVkGpu*,
                                  const GrVkRenderPass& compatibleRenderPass,
                                  const LoadStoreOps& colorOp,
                                  const LoadStoreOps& stencilOp);

    // The following return the index of the render pass attachment array for the given attachment.
    // If the render pass does not have the given attachment it will return false and not set the
    // index value.
    bool colorAttachmentIndex(uint32_t* index) const;
    bool stencilAttachmentIndex(uint32_t* index) const;
    bool hasStencilAttachment() const { return fAttachmentFlags & kStencil_AttachmentFlag; }

    bool hasSelfDependency() const { return fHasSelfDependency; }

    // Returns whether or not the structure of a RenderTarget matches that of the VkRenderPass in
    // this object. Specifically this compares that the number of attachments, format of
    // attachments, and sample counts are all the same. This function is used in the creation of
    // basic RenderPasses that can be used when creating a VkFrameBuffer object.
    bool isCompatible(const GrVkRenderTarget& target, bool hasInputSelfDependency) const;

    bool isCompatible(const GrVkRenderPass& renderPass) const;

    bool isCompatible(const AttachmentsDescriptor&,
                      const AttachmentFlags&,
                      bool hasInputSelfDependency) const;

    bool isCompatibleExternalRP(VkRenderPass) const;

    SkDEBUGCODE(bool isExternal() const { return fAttachmentFlags & kExternal_AttachmentFlag; })

    bool equalLoadStoreOps(const LoadStoreOps& colorOps,
                           const LoadStoreOps& stencilOps) const;

    VkRenderPass vkRenderPass() const { return fRenderPass; }

    const VkExtent2D& granularity() const { return fGranularity; }

    // Returns the number of clear colors needed to begin this render pass. Currently this will
    // either only be 0 or 1 since we only ever clear the color attachment.
    uint32_t clearValueCount() const { return fClearValueCount; }


    void genKey(GrProcessorKeyBuilder*) const;

    static void GenKey(GrProcessorKeyBuilder*,
                       AttachmentFlags,
                       const AttachmentsDescriptor&,
                       bool hasSelfDependency,
                       uint64_t externalRenderPass);

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkRenderPass: %d (%d refs)\n", fRenderPass, this->getRefCnt());
    }
#endif

private:
    GrVkRenderPass(const GrVkGpu*, VkRenderPass, AttachmentFlags, const AttachmentsDescriptor&,
                   bool hasInputSelfDependency, const VkExtent2D& granularity,
                   uint32_t clearValueCount);

    static GrVkRenderPass* Create(GrVkGpu* gpu,
                                  AttachmentFlags,
                                  AttachmentsDescriptor*,
                                  const LoadStoreOps& colorOps,
                                  const LoadStoreOps& stencilOps,
                                  bool needsSelfDependency);

    void freeGPUData() const override;

    VkRenderPass          fRenderPass;
    AttachmentFlags       fAttachmentFlags;
    AttachmentsDescriptor fAttachmentsDescriptor;
    bool                  fHasSelfDependency;
    VkExtent2D            fGranularity;
    uint32_t              fClearValueCount;
    // For internally created render passes we assume the color attachment index is always 0.
    uint32_t              fColorAttachmentIndex = 0;

    typedef GrVkManagedResource INHERITED;
};

GR_MAKE_BITFIELD_OPS(GrVkRenderPass::AttachmentFlags);

#endif
