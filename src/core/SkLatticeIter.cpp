/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLatticeIter.h"
#include "SkRect.h"

/**
 *  Divs must be in increasing order with no duplicates.
 */
static bool valid_divs(const int* divs, int count, int len) {
    if (count <= 0) {
        return false;
    }

    int prev = -1;
    for (int i = 0; i < count; i++) {
        if (prev >= divs[i] || divs[i] > len) {
            return false;
        }
    }

    return true;
}

bool SkLatticeIter::Valid(int width, int height, const SkCanvas::Lattice& lattice) {
    return valid_divs(lattice.fXDivs, lattice.fXCount, width) &&
           valid_divs(lattice.fYDivs, lattice.fYCount, height);
}

/**
 *  Count the number of pixels that are in "scalable" patches.
 */
static int count_scalable_pixels(const int32_t* divs, int numDivs, bool firstIsScalable,
                                 int length) {
    if (0 == numDivs) {
        return firstIsScalable ? length : 0;
    }

    int i;
    int count;
    if (firstIsScalable) {
        count = divs[0];
        i = 1;
    } else {
        count = 0;
        i = 0;
    }

    for (; i < numDivs; i += 2) {
        // Alternatively, we could use |top| and |bottom| as variable names, instead of
        // |left| and |right|.
        int left = divs[i];
        int right = (i + 1 < numDivs) ? divs[i + 1] : length;
        count += right - left;
    }

    return count;
}

/**
 *  Set points for the src and dst rects on subsequent draw calls.
 */
static void set_points(float* dst, float* src, const int* divs, int divCount, int srcFixed,
                       int srcScalable, float dstStart, float dstStop, bool isScalable) {

    float dstLen = dstStop - dstStart;
    int srcLen = srcFixed + srcScalable;
    float scale;
    if (srcFixed <= dstLen) {
        // This is the "normal" case, where we scale the "scalable" patches and leave
        // the other patches fixed.
        scale = (dstLen - ((float) srcFixed)) / ((float) srcScalable);
    } else {
        // In this case, we eliminate the "scalable" patches and scale the "fixed" patches.
        scale = dstLen / ((float) srcFixed);
    }

    src[0] = 0.0f;
    dst[0] = dstStart;
    for (int i = 0; i < divCount; i++) {
        src[i + 1] = (float) (divs[i]);
        float srcDelta = src[i + 1] - src[i];
        float dstDelta;
        if (srcFixed <= dstLen) {
            dstDelta = isScalable ? scale * srcDelta : srcDelta;
        } else {
            dstDelta = isScalable ? 0.0f : scale * srcDelta;
        }
        dst[i + 1] = dst[i] + dstDelta;

        // Alternate between "scalable" and "fixed" patches.
        isScalable = !isScalable;
    }

    src[divCount + 1] = (float) srcLen;
    dst[divCount + 1] = dstStop;
}

SkLatticeIter::SkLatticeIter(int srcWidth, int srcHeight, const SkCanvas::Lattice& lattice,
                             const SkRect& dst)
{
    const int* xDivs = lattice.fXDivs;
    int xCount = lattice.fXCount;
    const int* yDivs = lattice.fYDivs;
    int yCount = lattice.fYCount;

    // In the x-dimension, the first rectangle always starts at x = 0 and is "scalable".
    // If xDiv[0] is 0, it indicates that the first rectangle is degenerate, so the
    // first real rectangle "scalable" in the x-direction.
    //
    // The same interpretation applies to the y-dimension.
    //
    // As we move left to right across the image, alternating patches will be "fixed" or
    // "scalable" in the x-direction.  Similarly, as move top to bottom, alternating
    // patches will be "fixed" or "scalable" in the y-direction.
    SkASSERT(xCount > 0 && yCount > 0);
    bool xIsScalable = (0 == xDivs[0]);
    if (xIsScalable) {
        // Once we've decided that the first patch is "scalable", we don't need the
        // xDiv.  It is always implied that we start at zero.
        xDivs++;
        xCount--;
    }
    bool yIsScalable = (0 == yDivs[0]);
    if (yIsScalable) {
        // Once we've decided that the first patch is "scalable", we don't need the
        // yDiv.  It is always implied that we start at zero.
        yDivs++;
        yCount--;
    }

    // We never need the final xDiv/yDiv if it is equal to the width/height.  This is implied.
    if (xCount > 0 && srcWidth == xDivs[xCount - 1]) {
        xCount--;
    }
    if (yCount > 0 && srcHeight == yDivs[yCount - 1]) {
        yCount--;
    }

    // Count "scalable" and "fixed" pixels in each dimension.
    int xCountScalable = count_scalable_pixels(xDivs, xCount, xIsScalable, srcWidth);
    int xCountFixed = srcWidth - xCountScalable;
    int yCountScalable = count_scalable_pixels(yDivs, yCount, yIsScalable, srcHeight);
    int yCountFixed = srcHeight - yCountScalable;

    fSrcX.reset(xCount + 2);
    fDstX.reset(xCount + 2);
    set_points(fDstX.begin(), fSrcX.begin(), xDivs, xCount, xCountFixed, xCountScalable,
               dst.fLeft, dst.fRight, xIsScalable);

    fSrcY.reset(yCount + 2);
    fDstY.reset(yCount + 2);
    set_points(fDstY.begin(), fSrcY.begin(), yDivs, yCount, yCountFixed, yCountScalable,
               dst.fTop, dst.fBottom, yIsScalable);

    fCurrX = fCurrY = 0;
    fDone = false;
    fNumRects = (xCount + 1) * (yCount + 1);
}

bool SkLatticeIter::Valid(int width, int height, const SkIRect& center) {
    return !center.isEmpty() && SkIRect::MakeWH(width, height).contains(center);
}

SkLatticeIter::SkLatticeIter(int w, int h, const SkIRect& c, const SkRect& dst) {
    SkASSERT(SkIRect::MakeWH(w, h).contains(c));

    fSrcX.reset(4);
    fSrcY.reset(4);
    fDstX.reset(4);
    fDstY.reset(4);

    fSrcX[0] = 0;
    fSrcX[1] = SkIntToScalar(c.fLeft);
    fSrcX[2] = SkIntToScalar(c.fRight);
    fSrcX[3] = SkIntToScalar(w);

    fSrcY[0] = 0;
    fSrcY[1] = SkIntToScalar(c.fTop);
    fSrcY[2] = SkIntToScalar(c.fBottom);
    fSrcY[3] = SkIntToScalar(h);

    fDstX[0] = dst.fLeft;
    fDstX[1] = dst.fLeft + SkIntToScalar(c.fLeft);
    fDstX[2] = dst.fRight - SkIntToScalar(w - c.fRight);
    fDstX[3] = dst.fRight;

    fDstY[0] = dst.fTop;
    fDstY[1] = dst.fTop + SkIntToScalar(c.fTop);
    fDstY[2] = dst.fBottom - SkIntToScalar(h - c.fBottom);
    fDstY[3] = dst.fBottom;

    if (fDstX[1] > fDstX[2]) {
        fDstX[1] = fDstX[0] + (fDstX[3] - fDstX[0]) * c.fLeft / (w - c.width());
        fDstX[2] = fDstX[1];
    }

    if (fDstY[1] > fDstY[2]) {
        fDstY[1] = fDstY[0] + (fDstY[3] - fDstY[0]) * c.fTop / (h - c.height());
        fDstY[2] = fDstY[1];
    }

    fCurrX = fCurrY = 0;
    fDone = false;
    fNumRects = 9;
}

bool SkLatticeIter::next(SkRect* src, SkRect* dst) {
    if (fDone) {
        return false;
    }

    const int x = fCurrX;
    const int y = fCurrY;
    SkASSERT(x >= 0 && x < fSrcX.count() - 1);
    SkASSERT(y >= 0 && y < fSrcY.count() - 1);

    src->set(fSrcX[x], fSrcY[y], fSrcX[x + 1], fSrcY[y + 1]);
    dst->set(fDstX[x], fDstY[y], fDstX[x + 1], fDstY[y + 1]);
    if (fSrcX.count() - 1 == ++fCurrX) {
        fCurrX = 0;
        fCurrY += 1;
        if (fCurrY >= fSrcY.count() - 1) {
            fDone = true;
        }
    }
    return true;
}

void SkLatticeIter::mapDstScaleTranslate(const SkMatrix& matrix) {
    SkASSERT(matrix.isScaleTranslate());
    SkScalar tx = matrix.getTranslateX();
    SkScalar sx = matrix.getScaleX();
    for (int i = 0; i < fDstX.count(); i++) {
        fDstX[i] = fDstX[i] * sx + tx;
    }

    SkScalar ty = matrix.getTranslateY();
    SkScalar sy = matrix.getScaleY();
    for (int i = 0; i < fDstY.count(); i++) {
        fDstY[i] = fDstY[i] * sy + ty;
    }
}
