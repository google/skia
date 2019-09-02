/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPathRaw_DEFINED
#define SkPathRaw_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPathTypes.h"
#include "src/core/SkSpan.h"

class SkPathRaw {
public:
    SkSpan<SkPoint>   fPts;
    SkSpan<uint8_t>   fVerbs;
    SkSpan<SkScalar>  fConicWeights;

    SkRect                  fBounds;

    SkPathFillType          fFillType;
    SkPathConvexityType     fConvexity;
    SkPathSegmentMask       fSegmentMask;
};

class SkPathRaw_Editor {
    SkPathRaw*  fRaw;
    SkPoint*    fCurrPt;
    uint8_t*    fCurrVb;
    SkScalar*   fCurrW;

    void assertRoomFor(int ptCount, bool conic = false) const {
        SkASSERT(ptCount >= 1 && ptCount <= 2);
        SkASSERT(!conic || (conic && (ptCount == 2)));
        SkASSERT(fCurrPt + ptCount <= fRaw->fPts.end());
        SkASSERT(fCurrVb < fRaw->fVerbs.end());
    }

public:
    SkPathRaw_Editor(SkPathRaw* raw) : fRaw(raw) {
        fCurrPt = raw->fPts.begin();
        fCurrVb = raw->fVerbs.begin();
        fCurrW  = raw->fConicWeights.begin();
    }

    SkPathRaw_Editor& moveTo(SkPoint p) {
        this->assertRoomFor(1);
        *fCurrPt++ = p;
        *fCurrVb++ = kMove_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& lineTo(SkPoint p) {
        this->assertRoomFor(1);
        *fCurrPt++ = p;
        *fCurrVb++ = kLine_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& quadTo(SkPoint p0, SkPoint p1) {
        this->assertRoomFor(2);
        *fCurrPt++ = p0;
        *fCurrPt++ = p1;
        *fCurrVb++ = kQuad_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& conicTo(SkPoint p0, SkPoint p1, SkScalar w) {
        this->assertRoomFor(2, true);
        *fCurrPt++ = p0;
        *fCurrPt++ = p1;
        *fCurrW++  = w;
        *fCurrVb++ = kConic_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& cubicTo(SkPoint p0, SkPoint p1, SkPoint p2) {
        this->assertRoomFor(3);
        *fCurrPt++ = p0;
        *fCurrPt++ = p1;
        *fCurrPt++ = p2;
        *fCurrVb++ = kCubic_SkPathVerb;
        return *this;
    }

    SkPathRaw_Editor& close() {
        SkASSERT(fCurrVb < fRaw->fVerbs.end());
        *fCurrVb++ = kClose_SkPathVerb;
        return *this;
    }
};

#endif
