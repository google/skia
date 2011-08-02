
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

class GrStencilBuffer : public GrResource {
public:
    int width() const { return fWidth; }
    int height() const { return fHeight; }
    int bits() const { return fBits; }

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

protected:
    GrStencilBuffer(GrGpu* gpu, int width, int height, int bits)
        : GrResource(gpu)
        , fWidth(width)
        , fHeight(height)
        , fBits(bits)
        , fLastClip()
        , fLastClipWidth(-1)
        , fLastClipHeight(-1) {
    }

private:
    int fWidth;
    int fHeight;
    int fBits;

    GrClip     fLastClip;
    int        fLastClipWidth;
    int        fLastClipHeight;

    typedef GrResource INHERITED;
};

#endif
