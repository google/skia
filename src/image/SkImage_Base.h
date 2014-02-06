/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Base_DEFINED
#define SkImage_Base_DEFINED

#include "SkImage.h"

class SkImage_Base : public SkImage {
public:
    SkImage_Base(int width, int height) : INHERITED(width, height) {}

    virtual void onDraw(SkCanvas*, SkScalar x, SkScalar y, const SkPaint*) = 0;
    virtual void onDrawRectToRect(SkCanvas*, const SkRect* src,
                                  const SkRect& dst, const SkPaint*) = 0;

    // Default impl calls onDraw
    virtual bool onReadPixels(SkBitmap*, const SkIRect& subset) const;

    virtual const void* onPeekPixels(SkImageInfo*, size_t* /*rowBytes*/) const {
        return NULL;
    }

    virtual GrTexture* onGetTexture() { return NULL; }

    // return a read-only copy of the pixels. We promise to not modify them,
    // but only inspect them (or encode them).
    virtual bool getROPixels(SkBitmap*) const { return false; }

private:
    typedef SkImage INHERITED;
};

#endif
