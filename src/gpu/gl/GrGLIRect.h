
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLIRect_DEFINED
#define GrGLIRect_DEFINED

#include "GrGLInterface.h"

/**
 * Helper struct for dealing with the fact that Ganesh and GL use different
 * window coordinate systems (top-down vs bottom-up)
 */
struct GrGLIRect {
    GrGLint   fLeft;
    GrGLint   fBottom;
    GrGLsizei fWidth;
    GrGLsizei fHeight;

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

    // sometimes we have a GrIRect from the client that we
    // want to simultaneously make relative to GL's viewport
    // and convert from top-down to bottom-up.
    void setRelativeTo(const GrGLIRect& glRect,
                       int leftOffset,
                       int topOffset,
                       int width,
                       int height) {
        fLeft = glRect.fLeft + leftOffset;
        fWidth = width;
        fBottom = glRect.fBottom + (glRect.fHeight - topOffset - height);
        fHeight = height;

        GrAssert(fLeft >= 0);
        GrAssert(fWidth >= 0);
        GrAssert(fBottom >= 0);
        GrAssert(fHeight >= 0);
    }

    bool contains(const GrGLIRect& glRect) const {
        return fLeft <= glRect.fLeft &&
               fBottom <= glRect.fBottom &&
               fLeft + fWidth >=  glRect.fLeft + glRect.fWidth &&
               fBottom + fHeight >=  glRect.fBottom + glRect.fHeight;
    }

    void invalidate() {fLeft = fWidth = fBottom = fHeight = -1;}

    bool operator ==(const GrGLIRect& glRect) const {
        return 0 == memcmp(this, &glRect, sizeof(GrGLIRect));
    }

    bool operator !=(const GrGLIRect& glRect) const {return !(*this == glRect);}
};

#endif
