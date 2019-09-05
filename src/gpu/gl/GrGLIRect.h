/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrGLIRect_DEFINED
#define GrGLIRect_DEFINED

#include "include/core/SkRect.h"
#include "include/gpu/GrTypes.h"

/**
 * Helper struct for dealing with the fact that Ganesh and GL use different
 * window coordinate systems (top-down vs bottom-up)
 */
struct GrGLIRect {
    int fX;
    int fY;
    int fWidth;
    int fHeight;

    static GrGLIRect MakeRelativeTo(GrSurfaceOrigin org, int rtHeight, const SkIRect& devRect) {
        GrGLIRect glRect;
        glRect.setRelativeTo(org, rtHeight, devRect);
        return glRect;
    }

    static GrGLIRect MakeRelativeTo(GrSurfaceOrigin origin, int surfaceHeight, int leftOffset,
                                    int topOffset, int width, int height) {
        GrGLIRect glRect;
        glRect.setRelativeTo(origin, surfaceHeight, leftOffset, topOffset, width, height);
        return glRect;
    }

    /**
     *  cast-safe way to treat the rect as an array of (4) ints.
     */
    const int* asInts() const {
        return &fX;

        GR_STATIC_ASSERT(0 == offsetof(GrGLIRect, fX));
        GR_STATIC_ASSERT(4 == offsetof(GrGLIRect, fY));
        GR_STATIC_ASSERT(8 == offsetof(GrGLIRect, fWidth));
        GR_STATIC_ASSERT(12 == offsetof(GrGLIRect, fHeight));
        GR_STATIC_ASSERT(16 == sizeof(GrGLIRect));  // For an array of GrGLIRect.
    }
    int* asInts() { return &fX; }

    SkIRect asSkIRect() const { return SkIRect::MakeXYWH(fX, fY, fWidth, fHeight); }

    // sometimes we have a SkIRect from the client that we
    // want to simultaneously make relative to GL's viewport
    // and (optionally) convert from top-down to bottom-up.
    // The GL's viewport will always be the full size of the
    // current render target so we just pass in the rtHeight
    // here.
    void setRelativeTo(GrSurfaceOrigin org, int rtHeight, const SkIRect& devRect) {
        this->setRelativeTo(org, rtHeight, devRect.x(), devRect.y(), devRect.width(),
                            devRect.height());
    }

    void setRelativeTo(GrSurfaceOrigin origin, int surfaceHeight, int leftOffset, int topOffset,
                       int width, int height) {
        fX = leftOffset;
        fWidth = width;
        if (kBottomLeft_GrSurfaceOrigin == origin) {
            fY = surfaceHeight - topOffset - height;
        } else {
            fY = topOffset;
        }
        fHeight = height;

        SkASSERT(fWidth >= 0);
        SkASSERT(fHeight >= 0);
    }

    bool contains(int width, int height) const {
        return fX <= 0 &&
               fY <= 0 &&
               fX + fWidth >= width &&
               fY + fHeight >= height;
    }

    void invalidate() {fX = fWidth = fY = fHeight = -1;}
    bool isInvalid() const { return fX == -1 && fWidth == -1 && fY == -1
        && fHeight == -1; }

    bool operator ==(const GrGLIRect& glRect) const {
        return 0 == memcmp(this, &glRect, sizeof(GrGLIRect));
    }

    bool operator !=(const GrGLIRect& glRect) const {return !(*this == glRect);}
};

#endif
