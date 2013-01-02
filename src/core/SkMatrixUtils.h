/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixUtils_DEFINED
#define SkMatrixUtils_DEFINED

#include "SkMatrix.h"

/**
 *  Number of subpixel bits used in skia's bilerp.
 *  See SkBitmapProcState_procs.h and SkBitmapProcState_filter.h
 */
#define kSkSubPixelBitsForBilerp   4

/**
 *  Given a matrix and a src-rect, return true if the computed dst-rect would
 *  align such that there is a 1-to-1 coorspondence between src and dst pixels.
 *  This can be called by drawing code to see if drawBitmap can be turned into
 *  drawSprite (which is faster).
 *
 *  The "closeness" test is based on the subpixelBits parameter. Pass 0 for
 *  round-to-nearest behavior (e.g. nearest neighbor sampling). Pass the number
 *  of subpixel-bits to simulate filtering.
 */
bool SkTreatAsSprite(const SkMatrix&, const SkRect& src, unsigned subpixelBits);
    

#endif
