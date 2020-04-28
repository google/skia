/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMidpointContourParser_DEFINED
#define GrMidpointContourParser_DEFINED

#include "src/core/SkPathPriv.h"

// Parses out each contour in a path and tracks the midpoint. Example usage:
//
//   SkTPathContourParser parser;
//   while (parser.parseNextContour()) {
//       SkPoint midpoint = parser.currentMidpoint();
//       for (auto [verb, pts] : parser.currentContour()) {
//           ...
//       }
//   }
//
class GrMidpointContourParser {
public:
    GrMidpointContourParser(const SkPath& path)
            : fPath(path)
            , fVerbs(SkPathPriv::VerbData(fPath))
            , fNumRemainingVerbs(fPath.countVerbs())
            , fPoints(SkPathPriv::PointData(fPath)) {}

    // Advances the internal state to the next contour in the path. Returns false if there are no
    // more contours.
    bool parseNextContour() {
        bool hasGeometry = false;
        while (fVerbsIdx < fNumRemainingVerbs) {
            switch (uint8_t verb = fVerbs[fVerbsIdx]) {
                case SkPath::kMove_Verb:
                    if (!hasGeometry) {
                        fMidpoint = fPoints[fPtsIdx];
                        fMidpointWeight = 1;
                        this->advance();
                        ++fVerbsIdx;
                        ++fPtsIdx;
                        continue;
                    }
                    return true;

                static_assert(SkPath::kLine_Verb  == 1); case 1:
                static_assert(SkPath::kQuad_Verb  == 2); case 2:
                static_assert(SkPath::kConic_Verb == 3); case 3:
                static_assert(SkPath::kCubic_Verb == 4); case 4:
                    static constexpr int kPtsAdvance[] = {0, 1, 2, 2, 3};
                    fPtsIdx += kPtsAdvance[verb];
                    fMidpoint += fPoints[fPtsIdx - 1];
                    ++fMidpointWeight;
                    hasGeometry = true;
                    break;
            }
            ++fVerbsIdx;
        }
        return hasGeometry;
    }

    // Allows for iterating the current contour using a range-for loop.
    SkPathPriv::WalkPath currentContour() {
        return SkPathPriv::WalkPath(fVerbs, fVerbs + fVerbsIdx, fPoints);
    }

    SkPoint currentMidpoint() { return fMidpoint * (1.f / fMidpointWeight); }

private:
    void advance() {
        fVerbs += fVerbsIdx;
        fNumRemainingVerbs -= fVerbsIdx;
        fVerbsIdx = 0;
        fPoints += fPtsIdx;
        fPtsIdx = 0;
    }

    const SkPath& fPath;

    const uint8_t* fVerbs;
    int fNumRemainingVerbs = 0;
    int fVerbsIdx = 0;

    const SkPoint* fPoints;
    int fPtsIdx = 0;

    SkPoint fMidpoint;
    int fMidpointWeight;
};

#endif
