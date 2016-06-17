/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNinePatchIter_DEFINED
#define SkNinePatchIter_DEFINED

#include "SkScalar.h"

struct SkIRect;
struct SkRect;

/**
 *  Disect a ninepatch request into an sequence of src-rect / dst-rect pairs
 */
class SkNinePatchIter {
public:
    static bool Valid(int imageWidth, int imageHeight, const SkIRect& center);

    SkNinePatchIter(int imageWidth, int imageHeight, const SkIRect& center, const SkRect& dst);

    /**
     *  While it returns true, use src/dst to draw the image/bitmap
     */
    bool next(SkRect* src, SkRect* dst);

private:
    SkScalar fSrcX[4];
    SkScalar fSrcY[4];
    SkScalar fDstX[4];
    SkScalar fDstY[4];

    int fCurrX;
    int fCurrY;
    bool fDone;
};

#endif
