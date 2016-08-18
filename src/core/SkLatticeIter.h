/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLatticeIter_DEFINED
#define SkLatticeIter_DEFINED

#include "SkCanvas.h"
#include "SkScalar.h"
#include "SkTArray.h"

struct SkIRect;
struct SkRect;

/**
 *  Disect a lattice request into an sequence of src-rect / dst-rect pairs
 */
class SkLatticeIter {
public:

    static bool Valid(int imageWidth, int imageHeight, const SkCanvas::Lattice& lattice);

    SkLatticeIter(int imageWidth, int imageHeight, const SkCanvas::Lattice& lattice,
                  const SkRect& dst);

    static bool Valid(int imageWidth, int imageHeight, const SkIRect& center);

    SkLatticeIter(int imageWidth, int imageHeight, const SkIRect& center, const SkRect& dst);

    /**
     *  While it returns true, use src/dst to draw the image/bitmap
     */
    bool next(SkRect* src, SkRect* dst);

    /**
     *  Apply a matrix to the dst points.
     */
    void mapDstScaleTranslate(const SkMatrix& matrix);

    /**
     *  Returns the total number of rects that will be drawn.
     */
    int numRects() const {
        return fNumRects;
    }

private:
    SkTArray<SkScalar> fSrcX;
    SkTArray<SkScalar> fSrcY;
    SkTArray<SkScalar> fDstX;
    SkTArray<SkScalar> fDstY;

    int  fCurrX;
    int  fCurrY;
    bool fDone;
    int  fNumRects;
};

#endif
