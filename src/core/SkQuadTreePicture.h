/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkQuadTreePicture_DEFINED
#define SkQuadTreePicture_DEFINED

#include "SkPicture.h"
#include "SkRect.h"

/**
 * Subclass of SkPicture that override the behavior of the
 * kOptimizeForClippedPlayback_RecordingFlag by creating an SkQuadGrid
 * structure rather than an R-Tree. The quad tree has generally faster
 * tree creation time, but slightly slower query times, as compared to
 * R-Tree, so some cases may be faster and some cases slower.
 */
class SK_API SkQuadTreePicture : public SkPicture {
public:
    SkQuadTreePicture(const SkIRect& bounds) : fBounds(bounds) {}
    virtual SkBBoxHierarchy* createBBoxHierarchy() const SK_OVERRIDE;
private:
    SkIRect fBounds;
};

#endif
