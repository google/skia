/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkPathRaw.h"
#include "src/core/SkSpan.h"

class SkPathRaw_Rect : public SkPathRaw {
    SkPoint fStoragePts[4];
    uint8_t fStorageVbs[5];

public:
    SkPathRaw_Rect() {
        fPts   = {fStoragePts, SK_ARRAY_COUNT(fStoragePts)};
        fVerbs = {fStorageVbs, SK_ARRAY_COUNT(fStorageVbs)};

        fVerbs[0] = kMove_SkPathVerb;
        fVerbs[1] = kLine_SkPathVerb;
        fVerbs[2] = kLine_SkPathVerb;
        fVerbs[3] = kLine_SkPathVerb;
        fVerbs[4] = kClose_SkPathVerb;
    }

    void setRect(const SkRect& r, SkPathDirection dir = kCW_SkPathDirection) {
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
};

class SkPathRaw_Oval : public SkPathRaw {
    SkPoint fStoragePts[9];
    uint8_t fStorageVbs[6];

public:
    SkPathRaw_Oval() {
        fPts   = {fStoragePts, SK_ARRAY_COUNT(fStoragePts)};
        fVerbs = {fStorageVbs, SK_ARRAY_COUNT(fStorageVbs)};
    }

    void setOval(const SkRect& r, SkPathDirection dir = kCW_SkPathDirection) {
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
};
