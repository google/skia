/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPerspIter_DEFINED
#define SkPerspIter_DEFINED

#include "SkFixed.h"
#include "SkMatrix.h"

class SkPerspIter {
public:
    /** Iterate a line through the matrix [x,y] ... [x+count-1, y].
        @param m    The matrix we will be iterating a line through
        @param x    The initial X coordinate to be mapped through the matrix
        @param y    The initial Y coordinate to be mapped through the matrix
        @param count The number of points (x,y) (x+1,y) (x+2,y) ... we will eventually map
    */
    SkPerspIter(const SkMatrix& m, SkScalar x, SkScalar y, int count);

    /** Return the buffer of [x,y] fixed point values we will be filling.
        This always returns the same value, so it can be saved across calls to
        next().
    */
    const SkFixed* getXY() const { return fStorage; }

    /** Return the number of [x,y] pairs that have been filled in the getXY() buffer.
        When this returns 0, the iterator is finished.
    */
    int next();

private:
    enum {
        kShift  = 4,
        kCount  = (1 << kShift)
    };
    const SkMatrix& fMatrix;
    SkFixed         fStorage[kCount * 2];
    SkFixed         fX, fY;
    SkScalar        fSX, fSY;
    int             fCount;
};

#endif
