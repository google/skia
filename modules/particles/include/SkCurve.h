/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkCurve_DEFINED
#define SkCurve_DEFINED

#include "SkScalar.h"

class SkFieldVisitor;
class SkRandom;

// TODO: Generalize this to a keyframed list of cubics

struct SkCurve {
    SkCurve(SkScalar c = 0.0f) {
        fRanged = false;
        fMin[0] = fMin[1] = fMin[2] = fMin[3] = c;
        fMax[0] = fMax[1] = fMax[2] = fMax[3] = c;
    }

    SkScalar eval(float x, SkRandom& random) const;
    void visitFields(SkFieldVisitor* v);

    bool fRanged;
    SkScalar fMin[4];
    SkScalar fMax[4]; // used if ranged
};

#endif // SkCurve_DEFINED
