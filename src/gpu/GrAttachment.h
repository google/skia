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

class GrAttachment : public GrSurface {
public:
    enum class UsageFlags : uint8_t {
        kStencil = 0x1,
    };
    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(UsageFlags);

    ~GrAttachment() override {
        // TODO: allow SB to be purged and detach itself from rts
    }

    UsageFlags supportedUsages() const { return fSupportedUsages; }

    int numSamples() const { return fSampleCnt; }

    bool hasPerformedInitialClear() const { return fHasPerformedInitialClear; }
    void markHasPerformedInitialClear() { fHasPerformedInitialClear = true; }

    // This unique key is used for attachments of the same dimensions, usage, and sample cnt which
    // are shared between multiple render targets at the same time. Only one usage flag may be
    // passed in.
    // TODO: Once attachments start having multiple usages, we'll need to figure out how to search
    // the cache for an attachment that simply contains the requested usage instead of equaling it.
    static void ComputeSharedUniqueAttachmentKey(SkISize dimensions,
                                                 UsageFlags requiredUsage,
                                                 int sampleCnt,
                                                 GrUniqueKey* key);

protected:
    GrAttachment(GrGpu* gpu, SkISize dimensions, UsageFlags supportedUsages, int sampleCnt,
                 GrProtected isProtected)
            : INHERITED(gpu, dimensions, isProtected)
            , fSupportedUsages(supportedUsages)
            , fSampleCnt(sampleCnt) {}

private:
    const char* getResourceType() const override { return "Stencil"; }

    UsageFlags fSupportedUsages;
    int fSampleCnt;
    bool fHasPerformedInitialClear = false;

    using INHERITED = GrSurface;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrAttachment::UsageFlags);

#endif
