/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrNativeRect_DEFINED
#define GrNativeRect_DEFINED

#include "include/core/SkRect.h"
#include "include/gpu/ganesh/GrTypes.h"
#include "include/private/base/SkAssert.h"

#include <cstddef>
#include <cstring>

/**
 * Helper struct for dealing with bottom-up surface origins (bottom-up instead of top-down).
 */
struct GrNativeRect {
    int fX;
    int fY;
    int fWidth;
    int fHeight;

    static GrNativeRect MakeRelativeTo(GrSurfaceOrigin origin, int rtHeight, SkIRect devRect) {
        GrNativeRect nativeRect;
        nativeRect.setRelativeTo(origin, rtHeight, devRect);
        return nativeRect;
    }

    static SkIRect MakeIRectRelativeTo(GrSurfaceOrigin origin, int rtHeight, SkIRect devRect) {
        return MakeRelativeTo(origin, rtHeight, devRect).asSkIRect();
    }

    /**
     *  cast-safe way to treat the rect as an array of (4) ints.
     */
    const int* asInts() const {
        return &fX;

        static_assert(0 == offsetof(GrNativeRect, fX));
        static_assert(4 == offsetof(GrNativeRect, fY));
        static_assert(8 == offsetof(GrNativeRect, fWidth));
        static_assert(12 == offsetof(GrNativeRect, fHeight));
        static_assert(16 == sizeof(GrNativeRect));  // For an array of GrNativeRect.
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

    bool operator ==(const GrNativeRect& that) const {
        return 0 == memcmp(this, &that, sizeof(GrNativeRect));
    }

    bool operator !=(const GrNativeRect& that) const {return !(*this == that);}
};

#endif
