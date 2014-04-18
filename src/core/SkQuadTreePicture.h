/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkQuadTreePicture_DEFINED
#define SkQuadTreePicture_DEFINED

#ifdef SK_SUPPORT_LEGACY_PICTURE_HEADERS
#include "SkBBHFactory.h"
#endif

#ifdef SK_SUPPORT_LEGACY_DERIVED_PICTURE_CLASSES

#include "SkPicture.h"
#include "SkRect.h"

/**
 * Subclass of SkPicture that creates an SkQuadGrid
 * structure. The quad tree has generally faster
 * tree creation time, but slightly slower query times, as compared to
 * R-Tree, so some cases may be faster and some cases slower.
 */
class SK_API SkQuadTreePicture : public SkPicture {
public:
    SkQuadTreePicture(const SkIRect& bounds) : fBounds(bounds) {}
    virtual SkBBoxHierarchy* createBBoxHierarchy() const SK_OVERRIDE;
private:
    SkIRect fBounds;

    typedef SkPicture INHERITED;
};

class SkQuadTreePictureFactory : public SkPictureFactory {
public:
    SkQuadTreePictureFactory() {}

    virtual SkPicture* create(int width, int height) SK_OVERRIDE {
        return SkNEW_ARGS(SkQuadTreePicture, (SkIRect::MakeWH(width, height)));
    }

private:
    typedef SkPictureFactory INHERITED;
};

#endif

#endif
