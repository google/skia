/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRawShapes_DEFINED
#define SkPathRawShapes_DEFINED

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "src/core/SkPathRaw.h"

class SkRRect;
struct SkRect;

/**
 *  These classes provide their own stack-based storage for path data, making them efficient
 *  alternatives to SkPath for known geometries, avoiding heap allocations.
 *
 *  The defaults were chosen to match those in SkPathBuilder.h
 */
namespace SkPathRawShapes {

struct Rect : public SkPathRaw {
    SkPoint fStorage[4];   // move + 3 lines (+ close)

    Rect(const SkRect&,
         SkPathDirection = SkPathDirection::kCW,
         unsigned index = 0);
};

struct Oval : public SkPathRaw {
    SkPoint fStorage[9];   // move + 4 conics (+ close)

    Oval(const SkRect&,
         SkPathDirection = SkPathDirection::kCW,
         unsigned index = 1);
};

struct RRect : public SkPathRaw {
    SkPoint fStorage[13];   // worse case: move + 4 conics + 4 lines (+ close)

    RRect(const SkRRect&, SkPathDirection dir, unsigned index);
    RRect(const SkRRect& rr, SkPathDirection dir)
        : RRect(rr, dir, (dir == SkPathDirection::kCW ? 6 : 7)) {}
    RRect(const SkRRect& rr)
        : RRect(rr, SkPathDirection::kCW, 6) {}
};

struct Triangle : public SkPathRaw {
    Triangle(SkSpan<const SkPoint> threePoints, const SkRect& bounds);
};

}  // namespace SkPathRawShapes

#endif
