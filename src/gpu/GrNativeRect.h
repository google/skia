/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrNativeRect_DEFINED
#define GrNativeRect_DEFINED

#include "include/core/SkRect.h"
#include "include/gpu/GrTypes.h"

/**
 * Helper struct for dealing with bottom-up surface origins (bottom-up instead of top-down).
 */
struct GrNativeRect {
    int fX;
    int fY;
    int fWidth;
    int fHeight;

    /**
     *  cast-safe way to treat the rect as an array of (4) ints.
     */
    const int* asInts() const {
        return &fX;

        GR_STATIC_ASSERT(0 == offsetof(GrNativeRect, fX));
        GR_STATIC_ASSERT(4 == offsetof(GrNativeRect, fY));
        GR_STATIC_ASSERT(8 == offsetof(GrNativeRect, fWidth));
        GR_STATIC_ASSERT(12 == offsetof(GrNativeRect, fHeight));
        GR_STATIC_ASSERT(16 == sizeof(GrNativeRect));  // For an array of GrNativeRect.
    }
    int* asInts() { return &fX; }

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
        fX = leftOffset;
        fWidth = width;
        if (kBottomLeft_GrSurfaceOrigin == origin) {
            fY = fullHeight - topOffset - height;
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

    bool operator ==(const GrNativeRect& that) const {
        return 0 == memcmp(this, &that, sizeof(GrNativeRect));
    }

    bool operator !=(const GrNativeRect& that) const {return !(*this == that);}
};

#endif
