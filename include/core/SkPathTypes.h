/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathTypes_DEFINED
#define SkPathTypes_DEFINED

#include "SkTypes.h"

enum SkPathFillType {
    /** Specifies that "inside" is computed by a non-zero sum of signed edge crossings */
    kWinding_SkPathFillType,
    /** Specifies that "inside" is computed by an odd number of edge crossings */
    kEvenOdd_SkPathFillType,
    /** Same as Winding, but draws outside of the path, rather than inside */
    kInverseWinding_SkPathFillType,
    /** Same as EvenOdd, but draws outside of the path, rather than inside */
    kInverseEvenOdd_SkPathFillType
};

static inline bool SkPathFillTypeIsInverse(SkPathFillType ft) {
    return SkToBool(ft & 2);
}

enum SkPathConvexityType {
    kUnknown_SkPathConvexityType,
    kConvex_SkPathConvexityType,
    kConcave_SkPathConvexityType
};

enum SkPathDirection {
    /** Direction either has not been or could not be computed */
    kUnknown_SkPathDirection,
    /** clockwise direction for adding closed contours */
    kCW_SkPathDirection,
    /** counter-clockwise direction for adding closed contours */
    kCCW_SkPathDirection,
};

enum SkPathSegmentMask {
    kLine_SkPathSegmentMask   = 1 << 0,
    kQuad_SkPathSegmentMask   = 1 << 1,
    kConic_SkPathSegmentMask  = 1 << 2,
    kCubic_SkPathSegmentMask  = 1 << 3,
};

enum SkPathVerb {
    kMove_SkPathVerb,   //!< iter.next returns 1 point
    kLine_SkPathVerb,   //!< iter.next returns 2 points
    kQuad_SkPathVerb,   //!< iter.next returns 3 points
    kConic_SkPathVerb,  //!< iter.next returns 3 points + iter.conicWeight()
    kCubic_SkPathVerb,  //!< iter.next returns 4 points
    kClose_SkPathVerb,  //!< iter.next returns 1 point (contour's moveTo pt)
    kDone_SkPathVerb,   //!< iter.next returns 0 points
};

#endif
