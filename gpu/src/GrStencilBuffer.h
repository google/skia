
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencilBuffer_DEFINED
#define GrStencilBuffer_DEFINED

#include "GrClip.h"
#include "GrResource.h"

class GrRenderTarget;
class GrResourceEntry;

class GrStencilBuffer : public GrResource {
public:
    virtual ~GrStencilBuffer() {
        // currently each rt that has attached this sb keeps a ref
        // TODO: allow SB to be purged and detach itself from rts
        GrAssert(0 == fRTAttachmentCnt);
    }

    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int bits() const { return fBits; }
    int numSamples() const { return fSampleCnt; }

    // called to note the last clip drawn to this buffer.
    void setLastClip(const GrClip& clip, int width, int height) {
        fLastClip = clip;
        fLastClipWidth = width;
        fLastClipHeight = height;
        GrAssert(width <= fWidth);
        GrAssert(height <= fHeight);
    }

    // called to determine if we have to render the clip into SB.
    bool mustRenderClip(const GrClip& clip, int width, int height) const {
        // The clip is in device space. That is it doesn't scale to fit a
        // smaller RT. It is just truncated on the right / bottom edges.
        // Note that this assumes that the viewport origin never moves within
        // the stencil buffer. This is valid today.
        return width > fLastClipWidth ||
               height > fLastClipHeight ||
               clip != fLastClip;
    }

    const GrClip& getLastClip() const {
        return fLastClip;
    }

    // places the sb in the cache and locks it. Caller transfers
    // a ref to the the cache which will unref when purged.
    void transferToCacheAndLock();

    void wasAttachedToRenderTarget(const GrRenderTarget* rt) {
        ++fRTAttachmentCnt;
    }

    void wasDetachedFromRenderTarget(const GrRenderTarget* rt);

protected:
    GrStencilBuffer(GrGpu* gpu, int width, int height, int bits, int sampleCnt)
        : GrResource(gpu)
        , fWidth(width)
        , fHeight(height)
        , fBits(bits)
        , fSampleCnt(sampleCnt)
        , fLastClip()
        , fLastClipWidth(-1)
        , fLastClipHeight(-1)
        , fCacheEntry(NULL)
        , fRTAttachmentCnt(0) {
    }

    // GrResource overrides

    // subclass override must call INHERITED::onRelease
    virtual void onRelease();
    // subclass override must call INHERITED::onAbandon
    virtual void onAbandon();

private:

    void unlockInCache();

    int fWidth;
    int fHeight;
    int fBits;
    int fSampleCnt;

    GrClip     fLastClip;
    int        fLastClipWidth;
    int        fLastClipHeight;

    GrResourceEntry* fCacheEntry;
    int              fRTAttachmentCnt;

    typedef GrResource INHERITED;
};

#endif
