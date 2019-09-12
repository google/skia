/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPathMakers.h"
#include "src/core/SkPathRaw.h"
#include "src/core/SkSpan.h"

void SkPathRaw_Editor::seal(SkPathConvexityType ct) {
    if (fRaw->fPts.size() == 0) {
        fRaw->fBounds.setEmpty();
        fRaw->fIsFinite = true;
        fRaw->fConvexity = kConvex_SkPathConvexityType;
        fRaw->fSegmentMask = (SkPathSegmentMask)0;
    } else {
        fRaw->fIsFinite = fRaw->fBounds.setBoundsCheck(fRaw->fPts.begin(),
                                                       SkToInt(fRaw->fPts.size()));
        if (ct == kUnknown_SkPathConvexityType) {
        //    SkPathPriv::ComputeConvexity(*this);
        } else {
            fRaw->fConvexity = ct;
        }
        fRaw->fSegmentMask = (SkPathSegmentMask)fSegmentMask;
    }
}

SkPathRaw_Rect::SkPathRaw_Rect(const SkRect& oval, SkPathDirection dir) {
    fPts   = {fStoragePts, SK_ARRAY_COUNT(fStoragePts)};
    fVerbs = {fStorageVbs, SK_ARRAY_COUNT(fStorageVbs)};

    SkPath_RectPointIterator iter(oval, (SkPath::Direction)dir, (dir == kCW_SkPathDirection ? 0 : 1));

    SkPathRaw_Editor ed(this);
    ed.init(kEvenOdd_SkPathFillType);
    ed.moveTo(iter.current()).
       lineTo(iter.next()).
       lineTo(iter.next()).
       lineTo(iter.next()).
       close();
    ed.seal(kConvex_SkPathConvexityType);
}

SkPathRaw_Oval::SkPathRaw_Oval(const SkRect& oval, SkPathDirection dir) {
    fPts   = {fStoragePts, SK_ARRAY_COUNT(fStoragePts)};
    fVerbs = {fStorageVbs, SK_ARRAY_COUNT(fStorageVbs)};

    SkPath_OvalPointIterator ovalIter(oval, (SkPath::Direction)dir, 0);
    SkPath_RectPointIterator rectIter(oval, (SkPath::Direction)dir,
                                      (dir == kCW_SkPathDirection ? 0 : 1));
    const SkScalar weight = SK_ScalarRoot2Over2;

    SkPathRaw_Editor ed(this);
    ed.moveTo(ovalIter.current());
    for (unsigned i = 0; i < 4; ++i) {
        ed.conicTo(rectIter.next(), ovalIter.next(), weight);
    }
    ed.close();
}

SkPathRaw_Iter::SkPathRaw_Iter(const SkPathRaw& raw) {
    fMoveToPtr = fPts = raw.fPts.begin();
    fVerbs = raw.fVerbs.begin();
    fConicWeights = raw.fConicWeights.begin();
    if (fConicWeights) {
        fConicWeights -= 1;  // begin one behind
    }

    fNeedsCloseLine = false;

#ifdef SK_DEBUG
    fPtsEnd = raw.fPts.end();
    fVerbsEnd = raw.fVerbs.end();
    fIsConic = false;
#endif
}

///////////////////////

SkPathRaw::SkPathRaw(const SkPath& src) {
    fPts = { src.fPathRef->fPoints.begin(), (size_t)src.fPathRef->fPoints.count() };
    fVerbs = { src.fPathRef->fVerbs.begin(), (size_t)src.fPathRef->fVerbs.count() };
    fConicWeights = {
        src.fPathRef->fConicWeights.begin(), (size_t)src.fPathRef->fConicWeights.count()
    };
    fBounds = src.getBounds();
    fFillType = (SkPathFillType)src.getFillType();
    fConvexity = (SkPathConvexityType)src.getConvexity();
    fSegmentMask = (SkPathSegmentMask)src.getSegmentMasks();
    fIsFinite = src.isFinite();
}
