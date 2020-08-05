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

    bool isEmpty() const { return fPoints.size() == 0; }

    bool isRect(SkRect*) const;
    bool isFinite() const;  // TODO: cache this?

    SkSpan<const SkPoint> fPoints;
    SkSpan<const uint8_t> fVerbs;
    SkSpan<const float>   fWeights;

    SkRect fBounds;

    SkPathFillType      fFillType;
    SkPathConvexityType fConvexity;
    uint8_t             fSegmentMask;
};

struct SkSPath_triangle : public SkSPath {
    uint8_t fVerbs3[3];

    SkSPath_triangle(const SkPoint pts[3], const SkRect& bounds)
        : SkSPath({pts, 3}, {fVerbs3, 3}, {nullptr, 0},
                  SkPathFillType::kWinding, SkPathConvexityType::kConvex,
                  bounds, kLine_SkPathSegmentMask)
    {
        fVerbs3[0] = (uint8_t)SkPathVerb::kMove;
        fVerbs3[1] =
        fVerbs3[2] = (uint8_t)SkPathVerb::kLine;
    }
};

struct SkSPath_quad : public SkSPath {
    uint8_t fVerbs4[4];

    SkSPath_quad(const SkPoint pts[4], const SkRect& bounds)
        : SkSPath({pts, 4}, {fVerbs4, 4}, {nullptr, 0},
                  SkPathFillType::kWinding, SkPathConvexityType::kConvex,
                  bounds, kLine_SkPathSegmentMask)
    {
        fVerbs4[0] = (uint8_t)SkPathVerb::kMove;
        fVerbs4[1] =
        fVerbs4[2] =
        fVerbs4[3] = (uint8_t)SkPathVerb::kLine;
    }
};
