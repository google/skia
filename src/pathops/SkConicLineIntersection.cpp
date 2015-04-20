/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkIntersections.h"
#include "SkPathOpsConic.h"
#include "SkPathOpsLine.h"

class LineConicIntersections {
public:
    LineConicIntersections(const SkDConic& c, const SkDLine& l, SkIntersections* i)
        : fConic(c)
        , fLine(l)
        , fIntersections(i)
        , fAllowNear(true) {
        i->setMax(3);  // allow short partial coincidence plus discrete intersection
    }

    void allowNear(bool allow) {
        fAllowNear = allow;
    }

    int intersectRay(double roots[2]) {
        return 0;
    }
};

int SkIntersections::intersectRay(const SkDConic& conic, const SkDLine& line) {
    LineConicIntersections c(conic, line, this);
    fUsed = c.intersectRay(fT[0]);
    for (int index = 0; index < fUsed; ++index) {
        fPt[index] = conic.ptAtT(fT[0][index]);
    }
    return fUsed;
}
