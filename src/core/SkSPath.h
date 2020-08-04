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
    SkSPath(SkSpan<SkPoint> pts, SkSpan<uint8_t> verbs, SkSpan<float> weights,
            SkPathFillType ft, const SkRect& bounds, unsigned segmentMask)
        : fPoints(pts)
        , fVerbs(verbs)
        , fWeights(weights)
        , fBounds(bounds)
        , fFillType(ft)
        , fSegmentMask(segmentMask)
    {}

    // Computes bounds and segmentMask
    //
    SkSPath(SkSpan<SkPoint> pts, SkSpan<uint8_t> verbs, SkSpan<float> weights, SkPathFillType ft);

    SkSpan<SkPoint> fPoints;
    SkSpan<uint8_t> fVerbs;
    SkSpan<float>   fWeights;

    SkRect          fBounds;

    SkPathFillType  fFillType;
    uint8_t         fSegmentMask;
};
