/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRaw_DEFINED
#define SkPathRaw_DEFINED

#include "include/core/SkPathIter.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSpan.h"
#include "src/core/SkPathEnums.h"   // SkPathConvexity

#include <array>
#include <cstddef>
#include <optional>

/**
 *  SkPathRaw is a non-owning, immutable view of the path geometry.
 *
 *  It allows us to have stack-allocated paths, see SkPathRawShapes.h
 *
 *  It is the responsibility of the creator to ensure that the spans in SkPathRaw point to valid
 *  data that outlives the SkPathRaw instance.
 */
struct SkPathRaw {
    SkSpan<const SkPoint>    fPoints;
    SkSpan<const SkPathVerb> fVerbs;
    SkSpan<const float>      fConics;
    SkRect                   fBounds;
    SkPathFillType           fFillType;
    SkPathConvexity          fConvexity;
    uint8_t                  fSegmentMask;  // See SkPath::SegmentMask

    SkSpan<const SkPoint> points() const { return fPoints; }
    SkSpan<const SkPathVerb> verbs() const { return fVerbs; }
    SkSpan<const float> conics() const { return fConics; }
    SkRect bounds() const { return fBounds; }
    SkPathFillType fillType() const { return fFillType; }
    SkPathConvexity convexity() const { return fConvexity; }
    unsigned segmentMasks() const { return fSegmentMask; }

    bool empty() const { return fVerbs.empty(); }
    bool isInverseFillType() const { return SkPathFillType_IsInverse(fFillType); }
    bool isKnownToBeConvex() const { return SkPathConvexity_IsConvex(fConvexity); }

    std::optional<SkRect> isRect() const;

    SkPathIter iter() const { return {fPoints, fVerbs, fConics}; }

    static SkPathRaw Empty(SkPathFillType ft = SkPathFillType::kDefault) {
        return {
            {}, {}, {}, SkRect::MakeEmpty(), ft, SkPathConvexity::kConvex_Degenerate, 0,
        };
    }
};

#endif
