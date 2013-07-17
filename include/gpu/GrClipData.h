/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "SkClipStack.h"

class GrSurface;
struct SkIRect;

/**
 * GrClipData encapsulates the information required to construct the clip
 * masks. 'fOrigin' is only non-zero when saveLayer has been called
 * with an offset bounding box. The clips in 'fClipStack' are in
 * device coordinates (i.e., they have been translated by -fOrigin w.r.t.
 * the canvas' device coordinates).
 */
class GrClipData : public SkNoncopyable {
public:
    const SkClipStack*  fClipStack;
    SkIPoint            fOrigin;

    GrClipData()
        : fClipStack(NULL) {
        fOrigin.setZero();
    }

    bool operator==(const GrClipData& other) const {
        if (fOrigin != other.fOrigin) {
            return false;
        }

        if (NULL != fClipStack && NULL != other.fClipStack) {
            return *fClipStack == *other.fClipStack;
        }

        return fClipStack == other.fClipStack;
    }

    bool operator!=(const GrClipData& other) const {
        return !(*this == other);
    }

    void getConservativeBounds(const GrSurface* surface,
                               SkIRect* devResult,
                               bool* isIntersectionOfRects = NULL) const;
};

#endif
