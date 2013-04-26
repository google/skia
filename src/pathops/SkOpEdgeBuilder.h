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
#include "SkTDArray.h"

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
    bool close();
    int preFetch();
    bool walk();

    const SkPath* fPath;
    SkTDArray<SkPoint> fPathPts;
    SkTDArray<uint8_t> fPathVerbs;
    SkOpContour* fCurrentContour;
    SkTArray<SkOpContour>& fContours;
    SkTDArray<SkPoint> fReducePts;  // segments created on the fly
    SkTDArray<int> fExtra;  // -1 marks new contour, > 0 offsets into contour
    SkPathOpsMask fXorMask[2];
    const SkPoint* fFinalCurveStart;
    const SkPoint* fFinalCurveEnd;
    int fSecondHalf;
    bool fOperand;
    bool fAllowOpenContours;
    bool fUnparseable;
};

#endif
