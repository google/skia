/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrCCPRGeometry_DEFINED
#define GrGrCCPRGeometry_DEFINED

#include "SkTypes.h"

struct SkPoint;

/*
 * Ensures that a quadratic bezier is monotonic with respect to the vector between its endpoints
 * [P2 - P0]. In the event that the curve is not monotonic, it is chopped into two segments that
 * are. This should be rare for well-behaved curves in the real world.
 *
 * NOTE: This must be done in device space, since an affine transformation can change whether a
 * curve is monotonic.
 *
 * Returns false if the curve was already monotonic.
 *         true if it was chopped into two monotonic segments, now contained in dst.
 */
bool GrCCPRChopMonotonicQuadratics(const SkPoint& startPt, const SkPoint& controlPt,
                                   const SkPoint& endPt, SkPoint dst[5]);

#endif
