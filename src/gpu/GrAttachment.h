/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrMtlAttachment_DEFINED
#define GrMtlAttachment_DEFINED

#include "src/core/SkClipStack.h"
#include "src/gpu/GrSurface.h"

class GrRenderTarget;
class GrResourceKey;

class GrAttachment : public GrSurface {
public:
    ~GrAttachment() override {
        // TODO: allow SB to be purged and detach itself from rts
    }

    int numSamples() const { return fSampleCnt; }

    bool hasPerformedInitialClear() const { return fHasPerformedInitialClear; }
    void markHasPerformedInitialClear() { fHasPerformedInitialClear = true; }

    // We create a unique stencil buffer at each width, height and sampleCnt and share it for
    // all render targets that require a stencil with those params.
    static void ComputeSharedStencilAttachmentKey(SkISize dimensions, int sampleCnt,
                                                  GrUniqueKey* key);

protected:
    GrAttachment(GrGpu* gpu, SkISize dimensions, int sampleCnt, GrProtected isProtected)
            : INHERITED(gpu, dimensions, isProtected)
            , fSampleCnt(sampleCnt) {
    }

private:
    const char* getResourceType() const override { return "Stencil"; }

    int fSampleCnt;
    bool fHasPerformedInitialClear = false;

    using INHERITED = GrSurface;
};

#endif
