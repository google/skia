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

#include <array>
#include <cstddef>
#include <optional>

struct SkPathRaw {
    SkSpan<const SkPoint>    fPoints;
    SkSpan<const SkPathVerb> fVerbs;
    SkSpan<const float>      fConics;
    SkRect                   fBounds;
    SkPathFillType           fFillType;
    bool                     fIsConvex;
    uint8_t                  fSegmentMask;

    SkSpan<const SkPoint> points() const { return fPoints; }
    SkSpan<const SkPathVerb> verbs() const { return fVerbs; }
    SkSpan<const float> conics() const { return fConics; }
    SkRect bounds() const { return fBounds; }
    SkPathFillType fillType() const { return fFillType; }
    bool isConvex() const { return fIsConvex; }
    unsigned segmentMasks() const { return fSegmentMask; }

    bool empty() const { return fVerbs.empty(); }
    bool isInverseFillType() const { return SkPathFillType_IsInverse(fFillType); }

    std::optional<SkRect> isRect() const;

    SkPathIter iter() const { return {fPoints, fVerbs, fConics}; }
};

#endif
