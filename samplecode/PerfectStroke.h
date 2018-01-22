/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _PERFECT_STROKE_H_
#define _PERFECT_STROKE_H_

#include "SkOpContour.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkTArray.h"
#include "SkTDArray.h"

struct CurveData;

struct PathPair {
    SkRect fBounds;
    int fStart;  // index of point in path starting partial bounds
    int fEnd;  // index of point in path ending partial bounds
    PathPair* fPart1; // if fPath contains more than one segment, split here
    PathPair* fPart2;
};

enum class StrokeType {
    kInner,
    kOuter,
};

// todo: add array of intersections, ala pathops
//    index to segment in stroke path, t in segment; opp stroke, index, t; coincident
struct StrokeData {
    SkPath fPath;  // whole stroke
    PathPair fBoundsTree;
    SkPoint fBisect;   // mid angle formed by prev, this segs
    StrokeType fStrokeType;

    CurveData* curveData();
};

enum class CurvePos {
    kUnknown,
    kFirst,
    kMid,
    kLast,
};

// one per non-degenerate curve
struct CurveData {
    SkPoint fPts[4];
    SkScalar fWeight;
    SkPath::Verb fVerb;
    SkVector fTangents[2];
    CurveData* fPrev;
    StrokeData fInner;
    StrokeData fOuter;
    CurvePos fPos;
    SkOpContour fContour;

    const SkPoint& lastPt() const {
        SkASSERT(1 <= fVerb && fVerb <= 4);
        return fPts[(int) fVerb - (SkPath::kConic_Verb >= fVerb)];
    }

    // special case pair of lines with constant width
    void calcLineBisects(SkScalar width) {
        SkASSERT(SkPath::kLine_Verb == fPrev->fVerb);
        SkASSERT(SkPath::kLine_Verb == fVerb);
        SkVector lastV = fPrev->fTangents[1];
        SkVector nextV = fTangents[0];
        SkVector bisect = lastV + nextV;
        SkVector angleOpp = lastV - nextV;
        angleOpp.scale(0.5f);
        SkScalar oppLen = angleOpp.length();
        SkScalar oppSin = oppLen / width;
        bisect.setLength(width * width / oppLen);
        const SkPoint& pt = fPts[0];
        fInner.fBisect = pt + bisect;
        fOuter.fBisect = pt - bisect;
    }
};

struct StrokePair {
    SkRect fBounds;
    StrokePair* fPair1;
    StrokeData* fStroke1;
    StrokePair* fPair2;
    StrokeData* fStroke2;

    void validate() {
        SkASSERT(!!fPair1 == !fStroke1);
        SkASSERT(!!fPair2 == !fStroke2);
    }
};

struct StrokePath {
    SkPath fPath;
    SkTArray<CurveData> fCurveData;  // one per path segment
    SkTDArray<PathPair> fParts;  // storage for bounds of parts of strokes
    SkTDArray<StrokePair> fBranches;  // storage for branches of bounds tree
    StrokePair fBoundsTree;
    bool fLastBisectSet;

    void setPointArray(const SkPath& path, SkScalar width);
    void calcBisects(SkScalar width);
};

#endif
