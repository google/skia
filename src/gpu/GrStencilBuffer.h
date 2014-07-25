
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencilBuffer_DEFINED
#define GrStencilBuffer_DEFINED

#include "GrClipData.h"
#include "GrGpuResource.h"

class GrRenderTarget;
class GrResourceKey;

class GrStencilBuffer : public GrGpuResource {
public:
    SK_DECLARE_INST_COUNT(GrStencilBuffer);

    virtual ~GrStencilBuffer() {
        // TODO: allow SB to be purged and detach itself from rts
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int bits() const { return fBits; }
    int numSamples() const { return fSampleCnt; }

    // called to note the last clip drawn to this buffer.
    void setLastClip(int32_t clipStackGenID,
                     const SkIRect& clipSpaceRect,
                     const SkIPoint clipSpaceToStencilOffset) {
        fLastClipStackGenID = clipStackGenID;
        fLastClipStackRect = clipSpaceRect;
        fLastClipSpaceOffset = clipSpaceToStencilOffset;
    }

    // called to determine if we have to render the clip into SB.
    bool mustRenderClip(int32_t clipStackGenID,
                        const SkIRect& clipSpaceRect,
                        const SkIPoint clipSpaceToStencilOffset) const {
        return fLastClipStackGenID != clipStackGenID ||
               fLastClipSpaceOffset != clipSpaceToStencilOffset ||
               !fLastClipStackRect.contains(clipSpaceRect);
    }

    // Places the sb in the cache. The cache takes a ref of the stencil buffer.
    void transferToCache();

    static GrResourceKey ComputeKey(int width, int height, int sampleCnt);

protected:
    GrStencilBuffer(GrGpu* gpu, bool isWrapped, int width, int height, int bits, int sampleCnt)
        : GrGpuResource(gpu, isWrapped)
        , fWidth(width)
        , fHeight(height)
        , fBits(bits)
        , fSampleCnt(sampleCnt)
        , fLastClipStackGenID(SkClipStack::kInvalidGenID) {
        fLastClipStackRect.setEmpty();
    }

private:

    int fWidth;
    int fHeight;
    int fBits;
    int fSampleCnt;

    int32_t     fLastClipStackGenID;
    SkIRect     fLastClipStackRect;
    SkIPoint    fLastClipSpaceOffset;

    typedef GrGpuResource INHERITED;
};

#endif
