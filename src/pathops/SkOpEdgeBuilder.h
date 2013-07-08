/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkOpEdgeBuilder_DEFINED
#define SkOpEdgeBuilder_DEFINED

#include "SkOpContour.h"
#include "SkPathWriter.h"
#include "SkTArray.h"

class SkOpEdgeBuilder {
public:
    SkOpEdgeBuilder(const SkPathWriter& path, SkTArray<SkOpContour>& contours)
        : fPath(path.nativePath())
        , fContours(contours)
        , fAllowOpenContours(true) {
        init();
    }

    SkOpEdgeBuilder(const SkPath& path, SkTArray<SkOpContour>& contours)
        : fPath(&path)
        , fContours(contours)
        , fAllowOpenContours(false) {
        init();
    }

    void complete() {
        if (fCurrentContour && fCurrentContour->segments().count()) {
            fCurrentContour->complete();
            fCurrentContour = NULL;
        }
    }

    SkPathOpsMask xorMask() const {
        return fXorMask[fOperand];
    }

    void addOperand(const SkPath& path);
    bool finish();
    void init();

private:
    void closeContour(const SkPoint& curveEnd, const SkPoint& curveStart);
    bool close();
    int preFetch();
    bool walk();

    const SkPath* fPath;
    SkTArray<SkPoint, true> fPathPts;
    SkTArray<uint8_t, true> fPathVerbs;
    SkOpContour* fCurrentContour;
    SkTArray<SkOpContour>& fContours;
    SkPathOpsMask fXorMask[2];
    int fSecondHalf;
    bool fOperand;
    bool fAllowOpenContours;
    bool fUnparseable;
};

#endif
