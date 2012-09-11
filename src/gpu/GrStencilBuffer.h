
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencilBuffer_DEFINED
#define GrStencilBuffer_DEFINED

#include "GrClipData.h"
#include "GrResource.h"
#include "GrCacheID.h"

class GrRenderTarget;
class GrResourceEntry;
class GrResourceKey;

class GrStencilBuffer : public GrResource {
public:
    SK_DECLARE_INST_COUNT(GrStencilBuffer);
    GR_DECLARE_RESOURCE_CACHE_TYPE()

    virtual ~GrStencilBuffer() {
        // TODO: allow SB to be purged and detach itself from rts
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int bits() const { return fBits; }
    int numSamples() const { return fSampleCnt; }

    // called to note the last clip drawn to this buffer.
    void setLastClip(const GrClipData& clipData, int width, int height) {
        // the clip stack needs to be copied separately (and deeply) since
        // it could change beneath the stencil buffer
        fLastClipStack = *clipData.fClipStack;
        fLastClipData.fClipStack = &fLastClipStack;
        fLastClipData.fOrigin = clipData.fOrigin;
        fLastClipWidth = width;
        fLastClipHeight = height;
        GrAssert(width <= fWidth);
        GrAssert(height <= fHeight);
    }

    // called to determine if we have to render the clip into SB.
    bool mustRenderClip(const GrClipData& clipData, int width, int height) const {
        // The clip is in device space. That is it doesn't scale to fit a
        // smaller RT. It is just truncated on the right / bottom edges.
        // Note that this assumes that the viewport origin never moves within
        // the stencil buffer. This is valid today.
        return width > fLastClipWidth ||
               height > fLastClipHeight ||
               clipData != fLastClipData;
    }

    const GrClipData& getLastClip() const {
        return fLastClipData;
    }

    // Places the sb in the cache. The cache takes a ref of the stencil buffer.
    void transferToCache();

    static GrResourceKey ComputeKey(int width, int height, int sampleCnt);

protected:
    GrStencilBuffer(GrGpu* gpu, int width, int height, int bits, int sampleCnt)
        : GrResource(gpu)
        , fWidth(width)
        , fHeight(height)
        , fBits(bits)
        , fSampleCnt(sampleCnt)
        , fLastClipStack()
        , fLastClipData()
        , fLastClipWidth(-1)
        , fLastClipHeight(-1) {
    }

private:

    int fWidth;
    int fHeight;
    int fBits;
    int fSampleCnt;

    SkClipStack fLastClipStack;
    GrClipData  fLastClipData;
    int         fLastClipWidth;
    int         fLastClipHeight;

    typedef GrResource INHERITED;
};

#endif
