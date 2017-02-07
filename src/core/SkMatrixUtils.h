/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixUtils_DEFINED
#define SkMatrixUtils_DEFINED

#include "SkPoint.h"
#include "SkSize.h"

class SkMatrix;
class SkPaint;

/**
 *  Given a matrix, size and paint, return true if the computed dst-rect would
 *  align such that there is a 1-to-1 coorspondence between src and dst pixels.
 *  This can be called by drawing code to see if drawBitmap can be turned into
 *  drawSprite (which is faster).
 *
 *  The src-rect is defined to be { 0, 0, size.width(), size.height() }
 */
bool SkTreatAsSprite(const SkMatrix&, const SkISize& size, const SkPaint& paint);

/** Decomposes the upper-left 2x2 of the matrix into a rotation (represented by
    the cosine and sine of the rotation angle), followed by a non-uniform scale,
    followed by another rotation. If there is a reflection, one of the scale
    factors will be negative.
    Returns true if successful. Returns false if the matrix is degenerate.
    */
bool SkDecomposeUpper2x2(const SkMatrix& matrix,
                         SkPoint* rotation1,
                         SkPoint* scale,
                         SkPoint* rotation2);

#endif
