// Copyright 2023 Google LLC
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "modules/bentleyottmann/include/Contour.h"

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/private/base/SkTo.h"
#include "modules/bentleyottmann/include/Myers.h"

#include <algorithm>
#include <vector>

namespace contour {
Contours Contours::Make(SkPath path) {
    SkPath::Iter iter(path, false);
    Contours contours;
    while (auto rec = iter.next()) {
        SkSpan<const SkPoint> pts = rec->fPoints;
        switch (rec->fVerb) {
            case SkPathVerb::kConic: {
                SK_ABORT("Not implemented");
                break;
            }
            case SkPathVerb::kMove:
                contours.closeContourIfNeeded();
                contours.moveToStartOfContour(pts[0]);
                break;
            case SkPathVerb::kLine: {
                contours.addPointToCurrentContour(pts[1]);
                break;
            }
            case SkPathVerb::kQuad: {
                SK_ABORT("Not implemented");
                break;
            }
            case SkPathVerb::kCubic: {
                SK_ABORT("Not implemented");
                break;
            }
            case SkPathVerb::kClose: {
                contours.closeContourIfNeeded();
                break;
            }
        }
    }

    // Close the remaining contour.
    contours.closeContourIfNeeded();

    return contours;
}

std::vector<myers::Segment> Contours::segments() const {
    SK_ABORT("Not implemented");
}

static SkIRect extend_rect(SkIRect r, Point p) {
    int32_t left   = std::min(p.x, r.fLeft),
            top    = std::min(p.y, r.fTop),
            right  = std::max(p.x, r.fRight),
            bottom = std::max(p.y, r.fBottom);
    return {left, top, right, bottom};
}

Point Contours::RoundSkPoint(SkPoint p) {
    return {SkScalarRoundToInt(p.x() * kScaleFactor), SkScalarRoundToInt(p.y() * kScaleFactor)};
}

bool Contours::currentContourIsEmpty() const {
    int32_t lastEnd = fContours.empty() ? 0 : fContours.back().end;
    return lastEnd == SkToS32(fPoints.size());
}

void Contours::addPointToCurrentContour(SkPoint p) {
    if (this->currentContourIsEmpty()) {
        fPoints.push_back(fContourStart);
        fContourBounds = extend_rect(fContourBounds, fContourStart);
    }
    Point point = RoundSkPoint(p);
    fPoints.push_back(point);
    fContourBounds = extend_rect(fContourBounds, point);
}

void Contours::moveToStartOfContour(SkPoint p) {
    fContourStart = RoundSkPoint(p);
}

void Contours::closeContourIfNeeded() {
    if (this->currentContourIsEmpty()) {
        // The current contour is empty. Don't record it.
        return;
    }
    fContours.push_back({fContourBounds, SkToS32(fPoints.size())});
    fContourBounds = kEmptyRect;
}
}  // namespace contour
