/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkPath.h"

// region-inspired approach
void contourBounds(const SkPath& path, SkTDArray<SkRect>& boundsArray);
void simplify(const SkPath& path, bool asFill, SkPath& simple);

// contour outer edge walking approach
#ifndef DEFINE_SHAPE_OP
// FIXME: namespace testing doesn't allow global enums like this
#define DEFINE_SHAPE_OP
enum ShapeOp {
    kDifference_Op,
    kIntersect_Op,
    kUnion_Op,
    kXor_Op
};

enum ShapeOpMask {
    kWinding_Mask = -1,
    kNo_Mask = 0,
    kEvenOdd_Mask = 1
};
#endif

void operate(const SkPath& one, const SkPath& two, ShapeOp op, SkPath& result);
void simplifyx(const SkPath& path, SkPath& simple);

// FIXME: remove this section once debugging is complete
extern const bool gRunTestsInOneThread;
#ifdef SK_DEBUG
extern int gDebugMaxWindSum;
extern int gDebugMaxWindValue;
#endif
