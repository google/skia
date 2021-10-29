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

#include <cinttypes>

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
            , fSelfDepFlags(SelfDependencyFlags::kNone)
            , fLoadFromResolve(LoadFromResolve::kNo)
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
        AttachmentDesc fResolve;
        AttachmentDesc fStencil;
        uint32_t       fAttachmentCount;
    };

    enum AttachmentFlags {
        kColor_AttachmentFlag = 0x1,
        kStencil_AttachmentFlag = 0x2,
        kResolve_AttachmentFlag = 0x4,
        // The external attachment flag signals that this render pass is imported from an external
        // client. Since we don't know every attachment on the render pass we don't set any of the
        // specific attachment flags when using external. However, the external render pass must
        // at least have a color attachment.
        kExternal_AttachmentFlag = 0x8,
    };
    GR_DECL_BITFIELD_OPS_FRIENDS(AttachmentFlags);

    enum class SelfDependencyFlags {
        kNone =                   0,
        kForInputAttachment =     1 << 0,
        kForNonCoherentAdvBlend = 1 << 1,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(SelfDependencyFlags);

    enum class LoadFromResolve {
        kNo,
        kLoad,
    };

    static GrVkRenderPass* CreateSimple(GrVkGpu*,
                                        AttachmentsDescriptor*,
                                        AttachmentFlags,
                                        SelfDependencyFlags selfDepFlags,
                                        LoadFromResolve);
    static GrVkRenderPass* Create(GrVkGpu*,
                                  const GrVkRenderPass& compatibleRenderPass,
                                  const LoadStoreOps& colorOp,
                                  const LoadStoreOps& resolveOp,
                                  const LoadStoreOps& stencilOp);

    // The following return the index of the render pass attachment array for the given attachment.
    // If the render pass does not have the given attachment it will return false and not set the
    // index value.
    bool colorAttachmentIndex(uint32_t* index) const;
    bool stencilAttachmentIndex(uint32_t* index) const;
    bool hasStencilAttachment() const { return fAttachmentFlags & kStencil_AttachmentFlag; }
    bool hasResolveAttachment() const { return fAttachmentFlags & kResolve_AttachmentFlag; }

    SelfDependencyFlags selfDependencyFlags() const { return fSelfDepFlags; }
    LoadFromResolve loadFromResolve() const { return fLoadFromResolve; }

    // Returns whether or not the structure of a RenderTarget matches that of the VkRenderPass in
    // this object. Specifically this compares that the number of attachments, format of
    // attachments, and sample counts are all the same. This function is used in the creation of
    // basic RenderPasses that can be used when creating a VkFrameBuffer object.
    bool isCompatible(GrVkRenderTarget* target,
                      SelfDependencyFlags selfDepFlags,
                      LoadFromResolve) const;

    bool isCompatible(const GrVkRenderPass& renderPass) const;

    bool isCompatible(const AttachmentsDescriptor&,
                      const AttachmentFlags&,
                      SelfDependencyFlags selfDepFlags,
                      LoadFromResolve) const;

    bool isCompatibleExternalRP(VkRenderPass) const;

    SkDEBUGCODE(bool isExternal() const { return fAttachmentFlags & kExternal_AttachmentFlag; })

    bool equalLoadStoreOps(const LoadStoreOps& colorOps,
                           const LoadStoreOps& resolveOps,
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
                       SelfDependencyFlags selfDepFlags,
                       LoadFromResolve,
                       uint64_t externalRenderPass);

#ifdef SK_TRACE_MANAGED_RESOURCES
    void dumpInfo() const override {
        SkDebugf("GrVkRenderPass: %" PRIdPTR " (%d refs)\n",
                 (intptr_t)fRenderPass, this->getRefCnt());
    }
#endif

private:
    GrVkRenderPass(const GrVkGpu*, VkRenderPass, AttachmentFlags, const AttachmentsDescriptor&,
                   SelfDependencyFlags selfDepFlags, LoadFromResolve, const VkExtent2D& granularity,
                   uint32_t clearValueCount);

    static GrVkRenderPass* Create(GrVkGpu* gpu,
                                  AttachmentFlags,
                                  AttachmentsDescriptor*,
                                  const LoadStoreOps& colorOps,
                                  const LoadStoreOps& resolveOp,
                                  const LoadStoreOps& stencilOps,
                                  SelfDependencyFlags selfDepFlags,
                                  LoadFromResolve);

    void freeGPUData() const override;

    VkRenderPass          fRenderPass;
    AttachmentFlags       fAttachmentFlags;
    AttachmentsDescriptor fAttachmentsDescriptor;
    SelfDependencyFlags   fSelfDepFlags;
    LoadFromResolve       fLoadFromResolve;
    VkExtent2D            fGranularity;
    uint32_t              fClearValueCount;
    // For internally created render passes we assume the color attachment index is always 0.
    uint32_t              fColorAttachmentIndex = 0;
    uint32_t              fResolveAttachmentIndex = 0;

    using INHERITED = GrVkManagedResource;
};

GR_MAKE_BITFIELD_OPS(GrVkRenderPass::AttachmentFlags)
GR_MAKE_BITFIELD_CLASS_OPS(GrVkRenderPass::SelfDependencyFlags)

#endif
