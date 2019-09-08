/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPathRaw.h"
#include "src/core/SkSpan.h"

SkPathRaw_Rect::SkPathRaw_Rect(const SkRect& r, SkPathDirection dir) {
    fPts   = {fStoragePts, SK_ARRAY_COUNT(fStoragePts)};
    fVerbs = {fStorageVbs, SK_ARRAY_COUNT(fStorageVbs)};

    SkPathRaw_Editor ed(this);
    ed.moveTo({r.fLeft, r.fTop});
    if (dir == kCW_SkPathDirection) {
        ed.lineTo({r.fRight, r.fTop})
        .lineTo({r.fRight, r.fBottom})
        .lineTo({r.fLeft,  r.fBottom});
    } else {
        ed.lineTo({r.fLeft,  r.fBottom})
        .lineTo({r.fRight, r.fBottom})
        .lineTo({r.fRight, r.fTop});
    }
    ed.close();
}

SkPathRaw_Oval::SkPathRaw_Oval(const SkRect& r, SkPathDirection dir) {
    fPts   = {fStoragePts, SK_ARRAY_COUNT(fStoragePts)};
    fVerbs = {fStorageVbs, SK_ARRAY_COUNT(fStorageVbs)};

    const SkScalar mx = SkScalarAve(r.fLeft, r.fRight),
    my = SkScalarAve(r.fTop,  r.fBottom),
    w  = SK_ScalarRoot2Over2;

    SkPathRaw_Editor ed(this);
    ed.moveTo({mx, r.fTop});
    if (dir == kCW_SkPathDirection) {
        ed.conicTo({r.fRight, r.fTop},    {r.fRight, my},        w)
        .conicTo({r.fRight, r.fBottom}, {mx,       r.fBottom}, w)
        .conicTo({r.fLeft,  r.fBottom}, {r.fLeft,  my},        w)
        .conicTo({r.fLeft,  r.fTop},    {mx,       r.fTop},    w);
    } else {
        ed.conicTo({r.fLeft,  r.fTop},    {r.fLeft,  my},        w)
        .conicTo({r.fLeft,  r.fBottom}, {mx,       r.fBottom}, w)
        .conicTo({r.fRight, r.fBottom}, {r.fRight, my},        w)
        .conicTo({r.fRight, r.fTop},    {mx,       r.fTop},    w);
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
