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

struct SkSPath {
    SkSPath(SkSpan<const SkPoint> points, SkSpan<const uint8_t> verbs, SkSpan<const float> weights,
            SkPathFillType ft, SkPathConvexityType ct, const SkRect& bounds, unsigned segmentMask)
        : fPoints(points)
        , fVerbs(verbs)
        , fWeights(weights)
        , fBounds(bounds)
        , fFillType(ft)
        , fConvexity(ct)
        , fSegmentMask(segmentMask)
    {}

    // Computes bounds and segmentMask
    //
    SkSPath(SkSpan<const SkPoint> points, SkSpan<const uint8_t> verbs, SkSpan<const float> weights,
            SkPathFillType, SkPathConvexityType);

    bool isInverseFillType() const { return SkPathFillType_IsInverse(fFillType); }
    bool isConvex() const { return fConvexity == SkPathConvexityType::kConvex; }

    bool isRect(SkRect*) const { return false; }    // TODO needed?
    bool isFinite() const { return true; }          // TODO

    SkSpan<const SkPoint> fPoints;
    SkSpan<const uint8_t> fVerbs;
    SkSpan<const float>   fWeights;

    SkRect fBounds;

    SkPathFillType      fFillType;
    SkPathConvexityType fConvexity;
    uint8_t             fSegmentMask;
};

struct SkSPath_triangle : public SkSPath {
    uint8_t fTriVerbs[3];

    SkSPath_triangle(const SkPoint pts[], const SkRect& bounds)
        : SkSPath({pts, 3}, {fTriVerbs, 3}, {nullptr, 0},
                  SkPathFillType::kWinding, SkPathConvexityType::kConvex,
                  bounds, kLine_SkPathSegmentMask)
    {
        fTriVerbs[0] = (uint8_t)SkPathVerb::kMove;
        fTriVerbs[1] =
        fTriVerbs[2] = (uint8_t)SkPathVerb::kLine;
    }
};
