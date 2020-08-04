/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "src/core/SkSpan.h"

class SkSPath {
public:
    SkSPath(SkSpan<const SkPoint> points, SkSpan<const uint8_t> verbs, SkSpan<const float> weights,
            SkPathFillType ft, const SkRect& bounds, unsigned segmentMask)
        : fPoints(points)
        , fVerbs(verbs)
        , fWeights(weights)
        , fBounds(bounds)
        , fFillType(ft)
        , fSegmentMask(segmentMask)
    {}

    // Computes bounds and segmentMask
    //
    SkSPath(SkSpan<const SkPoint> points, SkSpan<const uint8_t> verbs, SkSpan<const float> weights,
            SkPathFillType);

    SkSpan<const SkPoint> fPoints;
    SkSpan<const uint8_t> fVerbs;
    SkSpan<const float>   fWeights;

    SkRect          fBounds;

    SkPathFillType  fFillType;
    uint8_t         fSegmentMask;
};
