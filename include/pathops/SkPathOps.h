/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkPathOps_DEFINED
#define SkPathOps_DEFINED

#include "SkPreConfig.h"

class SkPath;
struct SkRect;

// FIXME: move everything below into the SkPath class
/**
  *  The logical operations that can be performed when combining two paths.
  */
enum SkPathOp {
    kDifference_PathOp,         //!< subtract the op path from the first path
    kIntersect_PathOp,          //!< intersect the two paths
    kUnion_PathOp,              //!< union (inclusive-or) the two paths
    kXOR_PathOp,                //!< exclusive-or the two paths
    kReverseDifference_PathOp,  //!< subtract the first path from the op path
};

/** Set this path to the result of applying the Op to this path and the
    specified path: this = (this op operand).
    The resulting path will be constructed from non-overlapping contours.
    The curve order is reduced where possible so that cubics may be turned
    into quadratics, and quadratics maybe turned into lines.

    Returns true if operation was able to produce a result;
    otherwise, result is unmodified.

    @param one The first operand (for difference, the minuend)
    @param two The second operand (for difference, the subtrahend)
    @param result The product of the operands. The result may be one of the
                  inputs.
    @return True if operation succeeded.
  */
bool SK_API Op(const SkPath& one, const SkPath& two, SkPathOp op, SkPath* result);

/** Set this path to a set of non-overlapping contours that describe the
    same area as the original path.
    The curve order is reduced where possible so that cubics may
    be turned into quadratics, and quadratics maybe turned into lines.

    Returns true if operation was able to produce a result;
    otherwise, result is unmodified.

    @param path The path to simplify.
    @param result The simplified path. The result may be the input.
    @return True if simplification succeeded.
  */
bool SK_API Simplify(const SkPath& path, SkPath* result);

/** Set the resulting rectangle to the tight bounds of the path.

    @param path The path measured.
    @param result The tight bounds of the path.
    @return True if the bounds could be computed.
  */
bool SK_API TightBounds(const SkPath& path, SkRect* result);

#endif
