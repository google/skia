/*
 * Copyright 2022 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_tessellate_MidpointContourParser_DEFINED
#define skgpu_tessellate_MidpointContourParser_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "src/core/SkPathPriv.h"

#include <cstdint>

namespace skgpu::tess {

// Parses out each contour in a path and tracks the midpoint. Example usage:
//
//   MidpointContourParser parser;
//   while (parser.parseNextContour()) {
//       SkPoint midpoint = parser.currentMidpoint();
//       for (auto [verb, pts] : parser.currentContour()) {
//           ...
//       }
//   }
//
class MidpointContourParser {
public:
    MidpointContourParser(const SkPath& path)
            : fPath(path)
            , fVerbs(fPath.verbs().data())
            , fNumRemainingVerbs(fPath.countVerbs())
            , fPoints(fPath.points().data())
            , fWeights(fPath.conicWeights().data()) {}
    // Advances the internal state to the next contour in the path. Returns false if there are no
    // more contours.
    bool parseNextContour() {
        bool hasGeometry = false;
        for (; fVerbsIdx < fNumRemainingVerbs; ++fVerbsIdx) {
            switch (fVerbs[fVerbsIdx]) {
                case SkPathVerb::kMove:
                    if (!hasGeometry) {
                        fMidpoint = {0,0};
                        fMidpointWeight = 0;
                        this->advance();  // Resets fPtsIdx to 0 and advances fPoints.
                        fPtsIdx = 1;  // Increment fPtsIdx past the kMove.
                        continue;
                    }
                    if (fPoints[0] != fPoints[fPtsIdx - 1]) {
                        // There's an implicit close at the end. Add the start point to our mean.
                        fMidpoint += fPoints[0];
                        ++fMidpointWeight;
                    }
                    return true;
                default:
                    continue;
                case SkPathVerb::kLine:
                    ++fPtsIdx;
                    break;
                case SkPathVerb::kConic:
                    ++fWtsIdx;
                    [[fallthrough]];
                case SkPathVerb::kQuad:
                    fPtsIdx += 2;
                    break;
                case SkPathVerb::kCubic:
                    fPtsIdx += 3;
                    break;
            }
            fMidpoint += fPoints[fPtsIdx - 1];
            ++fMidpointWeight;
            hasGeometry = true;
        }
        if (hasGeometry && fPoints[0] != fPoints[fPtsIdx - 1]) {
            // There's an implicit close at the end. Add the start point to our mean.
            fMidpoint += fPoints[0];
            ++fMidpointWeight;
        }
        return hasGeometry;
    }

    // Allows for iterating the current contour using a range-for loop.
    SkPathPriv::Iterate currentContour() {
        return SkPathPriv::Iterate({fVerbs, (size_t)fVerbsIdx}, fPoints, fWeights);
    }

    SkPoint currentMidpoint() { return fMidpoint * (1.f / fMidpointWeight); }

private:
    void advance() {
        fVerbs += fVerbsIdx;
        fNumRemainingVerbs -= fVerbsIdx;
        fVerbsIdx = 0;
        fPoints += fPtsIdx;
        fPtsIdx = 0;
        fWeights += fWtsIdx;
        fWtsIdx = 0;
    }

    const SkPath& fPath;

    const SkPathVerb* fVerbs;
    int fNumRemainingVerbs = 0;
    int fVerbsIdx = 0;

    const SkPoint* fPoints;
    int fPtsIdx = 0;

    const float* fWeights;
    int fWtsIdx = 0;

    SkPoint fMidpoint;
    int fMidpointWeight;
};

}  // namespace skgpu::tess

#endif // skgpu_tessellate_MidpointContourParser_DEFINED
