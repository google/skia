/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathMeasurePriv_DEFINED
#define SkPathMeasurePriv_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "src/core/SkGeometry.h"

// Used in the Segment struct defined in SkPathMeasure.h
// It is used as a 2-bit field so if you add to this
// you must increase the size of the bitfield there.
enum SkSegType {
    kLine_SegType,
    kQuad_SegType,
    kCubic_SegType,
    kConic_SegType,
};


void SkPathMeasure_segTo(const SkPoint pts[], unsigned segType,
                   SkScalar startT, SkScalar stopT, SkPath* dst);

#endif  // SkPathMeasurePriv_DEFINED
