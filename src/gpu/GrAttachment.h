/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAttachment_DEFINED
#define GrAttachment_DEFINED

#include "src/core/SkClipStack.h"
#include "src/gpu/GrSurface.h"

class GrRenderTarget;
class GrResourceKey;

/**
 * This is a generic attachment class for out GrSurfaces. It always represents a single gpu
 * allocation. It contains usage flags so that we know what the attachment can be used for.
 *
 * TODO: Once we can pull out GrRenderTarget to be more of a framebuffer and break apart our
 * texture render target diamond, we will merge this class with GrSurface. Until then this will
 * act as the staging class for the new surface and framebuffer world.
 */
class GrAttachment : public GrSurface {
public:
    enum class UsageFlags : uint8_t {
        kStencilAttachment = 0x1,
        kColorAttachment   = 0x2,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(UsageFlags);

    ~GrAttachment() override {}

    UsageFlags supportedUsages() const { return fSupportedUsages; }

    int numSamples() const { return fSampleCnt; }

    bool hasPerformedInitialClear() const { return fHasPerformedInitialClear; }
    void markHasPerformedInitialClear() { fHasPerformedInitialClear = true; }

    // This unique key is used for attachments of the same dimensions, usage, and sample cnt which
    // are shared between multiple render targets at the same time. Only one usage flag may be
    // passed in.
    // TODO: Once attachments start having multiple usages, we'll need to figure out how to search
    // the cache for an attachment that simply contains the requested usage instead of equaling it.
    static void ComputeSharedAttachmentUniqueKey(const GrCaps& caps,
                                                 const GrBackendFormat& format,
                                                 SkISize dimensions,
                                                 UsageFlags requiredUsage,
                                                 int sampleCnt,
                                                 GrProtected isProtected,
                                                 GrUniqueKey* key);

    // TODO: Once attachments start having multiple usages, we'll need to figure out how to search
    // the cache for an attachment that simply contains the requested usage instead of equaling it.
    static void ComputeScratchKey(const GrCaps& caps,
                                  const GrBackendFormat& format,
                                  SkISize dimensions,
                                  UsageFlags requiredUsage,
                                  int sampleCnt,
                                  GrProtected,
                                  GrScratchKey* key);

protected:
    GrAttachment(GrGpu* gpu, SkISize dimensions, UsageFlags supportedUsages, int sampleCnt,
                 GrProtected isProtected)
            : INHERITED(gpu, dimensions, isProtected)
            , fSupportedUsages(supportedUsages)
            , fSampleCnt(sampleCnt) {}

private:
    size_t onGpuMemorySize() const final;

    void computeScratchKey(GrScratchKey*) const final;

    const char* getResourceType() const override {
        // TODO: Once attachments can have multiple usages this needs to be updated
        switch (fSupportedUsages) {
            case (UsageFlags::kColorAttachment):
                return "ColorAttachment";
            case (UsageFlags::kStencilAttachment):
                return "StencilAttachment";
            default:
                SkUNREACHABLE;
        }
    }

    UsageFlags fSupportedUsages;
    int fSampleCnt;
    bool fHasPerformedInitialClear = false;

    using INHERITED = GrSurface;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrAttachment::UsageFlags);

#endif
