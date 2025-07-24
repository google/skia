/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRaw_DEFINED
#define SkPathRaw_DEFINED

#include "include/core/SkPathTypes.h" // IWYU pragma: keep
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

    struct IterRec {
        SkSpan<const SkPoint> pts;
        float                 w;
        SkPathVerb            vrb;
    };

    class Iter {
    public:
        Iter(const SkPathRaw& p) : fPoints(p.fPoints), fVerbs(p.fVerbs), fConics(p.fConics) {
            vIndex = pIndex = cIndex = 0;
        }

        Iter(SkSpan<const SkPoint> pts, SkSpan<const SkPathVerb> vbs, SkSpan<const float> cns)
            : fPoints(pts), fVerbs(vbs), fConics(cns) {
            vIndex = pIndex = cIndex = 0;
        }

        /*  Holds the current verb, and its associated points
         *  move:  pts[0]
         *  line:  pts[0..1]
         *  quad:  pts[0..2]
         *  conic: pts[0..2] w
         *  cubic: pts[0..3]
         *  close: pts[0..1] ... as if close were a line from pts[0] to pts[1]
         */
        std::optional<IterRec> next();

    private:
        size_t                   vIndex, pIndex, cIndex;
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
        std::array<SkPoint, 2>   fPointStorage;
    };

    Iter iter() const {
        return Iter(*this);
    }

    struct ContourRec {
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
    };

    class ContourIter {
    public:
        ContourIter(const SkPathRaw&);

        std::optional<ContourRec> next();

    private:
        SkSpan<const SkPoint>    fPoints;
        SkSpan<const SkPathVerb> fVerbs;
        SkSpan<const float>      fConics;
    };
};

#endif
