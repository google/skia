/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrix22_DEFINED
#define SkMatrix22_DEFINED

#include "include/core/SkPoint.h"

class SkMatrix;

/** Find the Givens matrix G, which is the rotational matrix
 *  that rotates the vector h to the positive hoizontal axis.
 *  G * h = [hypot(h), 0]
 *
 *  This is equivalent to
 *
 *  SkScalar r = h.length();
 *  SkScalar r_inv = r ? SkScalarInvert(r) : 0;
 *  h.scale(r_inv);
 *  G->setSinCos(-h.fY, h.fX);
 *
 *  but has better numerical stability by using (partial) hypot,
 *  and saves a multiply by not computing r.
 */
void SkComputeGivensRotation(const SkVector& h, SkMatrix* G);

#endif
