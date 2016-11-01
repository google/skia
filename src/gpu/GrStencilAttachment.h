/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencilAttachment_DEFINED
#define GrStencilAttachment_DEFINED

#include "GrGpuResource.h"
#include "SkClipStack.h"

class GrRenderTarget;
class GrResourceKey;

class GrStencilAttachment : public GrGpuResource {
public:


    virtual ~GrStencilAttachment() {
        // TODO: allow SB to be purged and detach itself from rts
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int bits() const { return fBits; }
    int numSamples() const { return fSampleCnt; }

    // We create a unique stencil buffer at each width, height and sampleCnt and share it for
    // all render targets that require a stencil with those params.
    static void ComputeSharedStencilAttachmentKey(int width, int height, int sampleCnt,
                                                  GrUniqueKey* key);

protected:
    GrStencilAttachment(GrGpu* gpu, int width, int height, int bits, int sampleCnt)
        : GrGpuResource(gpu)
        , fWidth(width)
        , fHeight(height)
        , fBits(bits)
        , fSampleCnt(sampleCnt) {
    }

private:

    int fWidth;
    int fHeight;
    int fBits;
    int fSampleCnt;

    typedef GrGpuResource INHERITED;
};

#endif
