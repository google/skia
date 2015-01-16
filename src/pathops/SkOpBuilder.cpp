/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPathOps.h"

void SkOpBuilder::add(const SkPath& path, SkPathOp op) {
    if (0 == fOps.count() && op != kUnion_PathOp) {
        fPathRefs.push_back() = SkPath();
        *fOps.append() = kUnion_PathOp;
    }
    fPathRefs.push_back() = path;
    *fOps.append() = op;
}

void SkOpBuilder::reset() {
    fPathRefs.reset();
    fOps.reset();
}

/* OPTIMIZATION: Union doesn't need to be all-or-nothing. A run of three or more convex
   paths with union ops could be locally resolved and still improve over doing the
   ops one at a time. */
bool SkOpBuilder::resolve(SkPath* result) {
    int count = fOps.count();
    bool allUnion = true;
    SkPath::Direction firstDir;
    for (int index = 0; index < count; ++index) {
        SkPath* test = &fPathRefs[index];
        if (kUnion_PathOp != fOps[index] || test->isInverseFillType()) {
            allUnion = false;
            break;
        }
        // If all paths are convex, track direction, reversing as needed.
        if (test->isConvex()) {
            SkPath::Direction dir;
            if (!test->cheapComputeDirection(&dir)) {
                allUnion = false;
                break;
            }
            if (index == 0) {
                firstDir = dir;
            } else if (firstDir != dir) {
                SkPath temp;
                temp.reverseAddPath(*test);
                *test = temp;
            }
            continue;
        }
        // If the path is not convex but its bounds do not intersect the others, simplify is enough.
        const SkRect& testBounds = test->getBounds();
        for (int inner = 0; inner < index; ++inner) {
            // OPTIMIZE: check to see if the contour bounds do not intersect other contour bounds?
            if (SkRect::Intersects(fPathRefs[inner].getBounds(), testBounds)) {
                allUnion = false;
                break;
            }
        }
    }
    if (!allUnion) {
        *result = fPathRefs[0];
        for (int index = 1; index < count; ++index) {
            if (!Op(*result, fPathRefs[index], fOps[index], result)) {
                reset();
                return false;
            }
        }
        reset();
        return true;
    }
    SkPath sum;
    for (int index = 0; index < count; ++index) {
        if (!Simplify(fPathRefs[index], &fPathRefs[index])) {
            reset();
            return false;
        }
        sum.addPath(fPathRefs[index]);
    }
    reset();
    return Simplify(sum, result);
}
