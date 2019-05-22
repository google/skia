/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLIRect_DEFINED
#define GrGLIRect_DEFINED

#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

/**
 * Helper struct for dealing with the fact that Ganesh and GL use different
 * window coordinate systems (top-down vs bottom-up)
 */
struct GrGLIRect {
    GrGLint   fLeft;
    GrGLint   fBottom;
    GrGLsizei fWidth;
    GrGLsizei fHeight;

    /**
     *  cast-safe way to treat the rect as an array of (4) ints.
     */
    const int* asInts() const {
        return &fLeft;

        GR_STATIC_ASSERT(0 == offsetof(GrGLIRect, fLeft));
        GR_STATIC_ASSERT(4 == offsetof(GrGLIRect, fBottom));
        GR_STATIC_ASSERT(8 == offsetof(GrGLIRect, fWidth));
        GR_STATIC_ASSERT(12 == offsetof(GrGLIRect, fHeight));
        GR_STATIC_ASSERT(16 == sizeof(GrGLIRect)); // For an array of GrGLIRect.
    }
    int* asInts() { return &fLeft; }

    void pushToGLViewport(const GrGLInterface* gl) const {
        GR_GL_CALL(gl, Viewport(fLeft, fBottom, fWidth, fHeight));
    }

    void pushToGLScissor(const GrGLInterface* gl) const {
        GR_GL_CALL(gl, Scissor(fLeft, fBottom, fWidth, fHeight));
    }

    void setFromGLViewport(const GrGLInterface* gl) {
        GR_STATIC_ASSERT(sizeof(GrGLIRect) == 4*sizeof(GrGLint));
        GR_GL_GetIntegerv(gl, GR_GL_VIEWPORT, (GrGLint*) this);
    }

    // sometimes we have a SkIRect from the client that we
    // want to simultaneously make relative to GL's viewport
    // and (optionally) convert from top-down to bottom-up.
    // The GL's viewport will always be the full size of the
    // current render target so we just pass in the rtHeight
    // here.
    void setRelativeTo(int rtHeight, const SkIRect& devRect, GrSurfaceOrigin org) {
        this->setRelativeTo(rtHeight, devRect.x(), devRect.y(), devRect.width(), devRect.height(),
                            org);
    }

    void setRelativeTo(int fullHeight,
                       int leftOffset,
                       int topOffset,
                       int width,
                       int height,
                       GrSurfaceOrigin origin) {
        fLeft = leftOffset;
        fWidth = width;
        if (kBottomLeft_GrSurfaceOrigin == origin) {
            fBottom = fullHeight - topOffset - height;
        } else {
            fBottom = topOffset;
        }
        fHeight = height;

        SkASSERT(fWidth >= 0);
        SkASSERT(fHeight >= 0);
    }

    bool contains(int width, int height) const {
        return fLeft <= 0 &&
               fBottom <= 0 &&
               fLeft + fWidth >= width &&
               fBottom + fHeight >= height;
    }

    void invalidate() {fLeft = fWidth = fBottom = fHeight = -1;}
    bool isInvalid() const { return fLeft == -1 && fWidth == -1 && fBottom == -1
        && fHeight == -1; }

    bool operator ==(const GrGLIRect& glRect) const {
        return 0 == memcmp(this, &glRect, sizeof(GrGLIRect));
    }

    bool operator !=(const GrGLIRect& glRect) const {return !(*this == glRect);}
};

#endif
