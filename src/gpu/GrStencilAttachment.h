/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencilAttachment_DEFINED
#define GrStencilAttachment_DEFINED

#include "src/core/SkClipStack.h"
#include "src/gpu/GrSurface.h"

class GrRenderTarget;
class GrResourceKey;

class GrStencilAttachment : public GrSurface {
public:
    ~GrStencilAttachment() override {
        // TODO: allow SB to be purged and detach itself from rts
    }

    int bits() const { return fBits; }
    int numSamples() const { return fSampleCnt; }

    bool hasPerformedInitialClear() const { return fHasPerformedInitialClear; }
    void markHasPerformedInitialClear() { fHasPerformedInitialClear = true; }

    // We create a unique stencil buffer at each width, height and sampleCnt and share it for
    // all render targets that require a stencil with those params.
    static void ComputeSharedStencilAttachmentKey(SkISize dimensions, int sampleCnt,
                                                  GrUniqueKey* key);

protected:
    GrStencilAttachment(GrGpu* gpu, SkISize dimensions, int bits, int sampleCnt,
                        GrProtected isProtected)
            : INHERITED(gpu, dimensions, isProtected)
            , fBits(bits)
            , fSampleCnt(sampleCnt) {
    }

private:
    const char* getResourceType() const override { return "Stencil"; }

    int fBits;
    int fSampleCnt;
    bool fHasPerformedInitialClear = false;

    using INHERITED = GrSurface;
};

#endif
